/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define DT_DRV_COMPAT cros_x86_mtl_emulator

#include "../usb_request.h"
#include "ec_acok.h"
#include "ec_pwrbtn.h"

#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/minmax.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(x86_mtl_emulator, LOG_LEVEL_INF);

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
	     "only one 'cros,x86-mtl-emulator' compatible node may be present");

struct x86_mtl_emulator_config {
	/* Inputs from EC */
	struct gpio_dt_spec en_s5;
	struct gpio_dt_spec rsmrst_n;
	struct gpio_dt_spec pwrbtn_n;
	struct gpio_dt_spec sys_pwrok;
	struct gpio_dt_spec plt_pwrok;

	/* Outputs to EC */
	struct gpio_dt_spec rsmrst_pwrgd;
	struct gpio_dt_spec all_sys_pwrgd;
	struct gpio_dt_spec slp_s5;
	struct gpio_dt_spec slp_s4;
	struct gpio_dt_spec slp_s3;
	struct gpio_dt_spec slp_s0;
};

struct x86_mtl_emulator_data {
	const struct device *dev;

	struct gpio_callback cb_en_s5;
	struct gpio_callback cb_rsmrst_n;
	struct gpio_callback cb_pwrbtn_n;
	struct gpio_callback cb_plt_pwrok;

	enum ap_mode current_mode;
	enum ap_state current_state;

	struct k_work ap_work;
	struct k_work_delayable ap_poll_work;

	/* Previous logical pin states for edge detection */
	int prev_en_s5;
	int prev_rsmrst;
	int prev_pwrbtn;
	int prev_plt_pwrok;
	int prev_sys_pwrok;
	int prev_rsmrst_pwrgd;
	int prev_all_sys_pwrgd;
	int prev_slp_s3;
	bool plt_pwrok_seen_since_all_sys_pwrgd;
	int64_t s4_entry_ms;

	/* Hardware cycle timestamps of signal transitions */
	uint32_t rsmrst_pwrgd_assert_cyc;
	uint32_t all_sys_pwrgd_assert_cyc;
	uint32_t slp_s3_deassert_cyc;
	uint32_t slp_s3_assert_cyc;
	uint32_t rsmrst_deassert_cyc;
	uint32_t rsmrst_assert_cyc;
	uint32_t plt_pwrok_assert_cyc;
	uint32_t sys_pwrok_assert_cyc;
	uint32_t pwrbtn_assert_cyc;

	struct usb_ap_timings timings;
};

static const struct device *const emul_dev = DEVICE_DT_INST_GET(0);

static void ap_update_state_machine(const struct device *dev);

static uint32_t calc_elapsed_us(uint32_t start_cyc, uint32_t end_cyc)
{
	uint64_t us = k_cyc_to_us_floor64(end_cyc - start_cyc);

	return min(us, UINT32_MAX);
}

static uint32_t calc_elapsed_ns(uint32_t start_cyc, uint32_t end_cyc)
{
	uint64_t ns = k_cyc_to_ns_floor64(end_cyc - start_cyc);

	return min(ns, UINT32_MAX);
}

static bool ap_set_output_pin(const struct gpio_dt_spec *gpio, int val,
			      int *prev_val, uint32_t *assert_cyc,
			      uint32_t *deassert_cyc)
{
	int norm_val = val ? 1 : 0;

	gpio_pin_set_dt(gpio, norm_val);
	if (prev_val == NULL || norm_val == *prev_val) {
		return false;
	}

	uint32_t now = k_cycle_get_32();

	if (norm_val && assert_cyc != NULL) {
		*assert_cyc = now;
	} else if (!norm_val && deassert_cyc != NULL) {
		*deassert_cyc = now;
	}
	*prev_val = norm_val;

	return true;
}

static void ap_set_signal(const struct device *dev, enum ap_output_signal sig,
			  int val)
{
	const struct x86_mtl_emulator_config *cfg = dev->config;
	struct x86_mtl_emulator_data *data = dev->data;

	switch (sig) {
	case AP_OUT_RSMRST_PWRGD:
		ap_set_output_pin(&cfg->rsmrst_pwrgd, val,
				  &data->prev_rsmrst_pwrgd,
				  &data->rsmrst_pwrgd_assert_cyc, NULL);
		break;
	case AP_OUT_ALL_SYS_PWRGD:
		if (!ap_set_output_pin(&cfg->all_sys_pwrgd, val,
				       &data->prev_all_sys_pwrgd,
				       &data->all_sys_pwrgd_assert_cyc, NULL) ||
		    !val) {
			break;
		}
		data->plt_pwrok_seen_since_all_sys_pwrgd = false;
		data->timings.slp_s3_to_plt_pwrok_us = 0;
		data->timings.all_sys_pwrgd_to_plt_pwrok_us = 0;
		data->timings.all_sys_pwrgd_to_sys_pwrok_us = 0;
		data->timings.plt_pwrok_to_sys_pwrok_us = 0;
		break;
	case AP_OUT_SLP_S5:
		ap_set_output_pin(&cfg->slp_s5, val, NULL, NULL, NULL);
		break;
	case AP_OUT_SLP_S4:
		ap_set_output_pin(&cfg->slp_s4, val, NULL, NULL, NULL);
		break;
	case AP_OUT_SLP_S3:
		if (!ap_set_output_pin(&cfg->slp_s3, val, &data->prev_slp_s3,
				       &data->slp_s3_deassert_cyc,
				       &data->slp_s3_assert_cyc) ||
		    val) {
			break;
		}
		data->timings.slp_s3_assert_to_plt_pwrok_deassert_us = 0;
		break;
	case AP_OUT_SLP_S0:
		ap_set_output_pin(&cfg->slp_s0, val, NULL, NULL, NULL);
		break;
	default:
		break;
	}
}

static void ap_check_sys_pwrok(const struct device *dev, uint32_t now)
{
	const struct x86_mtl_emulator_config *cfg = dev->config;
	struct x86_mtl_emulator_data *data = dev->data;
	int sys_pwrok = gpio_pin_get_dt(&cfg->sys_pwrok);

	if (sys_pwrok == data->prev_sys_pwrok) {
		return;
	}
	data->prev_sys_pwrok = sys_pwrok;

	if (!sys_pwrok) {
		return;
	}

	data->sys_pwrok_assert_cyc = now;
	data->timings.all_sys_pwrgd_to_sys_pwrok_us =
		calc_elapsed_us(data->all_sys_pwrgd_assert_cyc, now);

	if (data->plt_pwrok_seen_since_all_sys_pwrgd) {
		data->timings.plt_pwrok_to_sys_pwrok_us =
			calc_elapsed_us(data->plt_pwrok_assert_cyc, now);
	} else {
		data->timings.plt_pwrok_to_sys_pwrok_us = 0;
	}
}

static void ap_work_handler(struct k_work *work)
{
	struct x86_mtl_emulator_data *data =
		CONTAINER_OF(work, struct x86_mtl_emulator_data, ap_work);

	ap_update_state_machine(data->dev);
}

static void ap_poll_work_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct x86_mtl_emulator_data *data =
		CONTAINER_OF(dwork, struct x86_mtl_emulator_data, ap_poll_work);

	ap_update_state_machine(data->dev);
}

static void rsmrst_n_callback(const struct device *port,
			      struct gpio_callback *cb, uint32_t pins)
{
	const struct x86_mtl_emulator_config *cfg = emul_dev->config;
	struct x86_mtl_emulator_data *data = emul_dev->data;
	uint32_t now = k_cycle_get_32();
	int rsmrst = gpio_pin_get_dt(&cfg->rsmrst_n);

	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	if (rsmrst != data->prev_rsmrst) {
		if (rsmrst) {
			data->rsmrst_assert_cyc = now;
		} else {
			data->rsmrst_deassert_cyc = now;
			data->timings.rsmrst_pwrgd_to_rsmrst_deassert_us =
				calc_elapsed_us(data->rsmrst_pwrgd_assert_cyc,
						now);
		}
		data->prev_rsmrst = rsmrst;
	}

	k_work_submit(&data->ap_work);
}

static void en_s5_callback(const struct device *port, struct gpio_callback *cb,
			   uint32_t pins)
{
	const struct x86_mtl_emulator_config *cfg = emul_dev->config;
	struct x86_mtl_emulator_data *data = emul_dev->data;
	uint32_t now = k_cycle_get_32();
	int en_s5 = gpio_pin_get_dt(&cfg->en_s5);

	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	if (en_s5 == data->prev_en_s5) {
		k_work_submit(&data->ap_work);
		return;
	}
	data->prev_en_s5 = en_s5;

	if (en_s5) {
		data->timings.rsmrst_assert_to_en_s5_deassert_ns = 0;
	} else if (data->prev_rsmrst) {
		data->timings.rsmrst_assert_to_en_s5_deassert_ns =
			calc_elapsed_ns(data->rsmrst_assert_cyc, now);
	} else {
		data->timings.rsmrst_assert_to_en_s5_deassert_ns = 0;
	}

	if (data->current_mode == AP_MODE_AUTO) {
		ap_set_signal(emul_dev, AP_OUT_RSMRST_PWRGD, en_s5);
	}

	k_work_submit(&data->ap_work);
}

static void plt_pwrok_callback(const struct device *port,
			       struct gpio_callback *cb, uint32_t pins)
{
	const struct x86_mtl_emulator_config *cfg = emul_dev->config;
	struct x86_mtl_emulator_data *data = emul_dev->data;
	uint32_t now = k_cycle_get_32();
	int plt_pwrok = gpio_pin_get_dt(&cfg->plt_pwrok);

	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	if (plt_pwrok != data->prev_plt_pwrok) {
		if (plt_pwrok) {
			data->plt_pwrok_assert_cyc = now;
			data->plt_pwrok_seen_since_all_sys_pwrgd = true;
			data->timings.slp_s3_to_plt_pwrok_us =
				calc_elapsed_us(data->slp_s3_deassert_cyc, now);
			data->timings.all_sys_pwrgd_to_plt_pwrok_us =
				calc_elapsed_us(data->all_sys_pwrgd_assert_cyc,
						now);
		} else {
			data->timings.slp_s3_assert_to_plt_pwrok_deassert_us =
				calc_elapsed_us(data->slp_s3_assert_cyc, now);
		}
		data->prev_plt_pwrok = plt_pwrok;
	}

	ap_check_sys_pwrok(emul_dev, now);
	k_work_submit(&data->ap_work);
}

static void pwrbtn_n_callback(const struct device *port,
			      struct gpio_callback *cb, uint32_t pins)
{
	const struct x86_mtl_emulator_config *cfg = emul_dev->config;
	struct x86_mtl_emulator_data *data = emul_dev->data;
	uint32_t now = k_cycle_get_32();
	int pwrbtn = gpio_pin_get_dt(&cfg->pwrbtn_n);

	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	if (pwrbtn != data->prev_pwrbtn) {
		if (pwrbtn) {
			data->pwrbtn_assert_cyc = now;
		} else {
			data->timings.pwrbtn_pulse_width_us =
				calc_elapsed_us(data->pwrbtn_assert_cyc, now);
		}
		data->prev_pwrbtn = pwrbtn;
	}

	k_work_submit(&data->ap_work);
}

static void ap_update_state_machine(const struct device *dev)
{
	const struct x86_mtl_emulator_config *cfg = dev->config;
	struct x86_mtl_emulator_data *data = dev->data;

	ap_check_sys_pwrok(dev, k_cycle_get_32());

	if (data->current_mode != AP_MODE_AUTO) {
		if (!data->prev_slp_s3 && data->prev_sys_pwrok) {
			k_work_reschedule(&data->ap_poll_work, K_USEC(250));
		}
		return;
	}

	int en_s5 = gpio_pin_get_dt(&cfg->en_s5);
	int rsmrst = gpio_pin_get_dt(&cfg->rsmrst_n);
	int sys_pwrok = gpio_pin_get_dt(&cfg->sys_pwrok);
	int pch_pwrok = gpio_pin_get_dt(&cfg->plt_pwrok);

	LOG_DBG("AP SM: en_s5=%d, rsmrst=%d, pch_pwrok=%d, sys_pwrok=%d, "
		"cur_state=%d",
		en_s5, rsmrst, pch_pwrok, sys_pwrok, data->current_state);

	/* Check for power-down or reset conditions */
	if (!en_s5 || rsmrst) {
		/* EN_S5 deasserted or RSMRST asserted -> reset back to G3 */
		if (data->current_state != AP_STATE_G3) {
			LOG_INF("AP SM: RSMRST asserted or EN_S5 deasserted -> "
				"G3");
			data->current_state = AP_STATE_G3;
			ap_set_signal(dev, AP_OUT_ALL_SYS_PWRGD, 0);
			ap_set_signal(dev, AP_OUT_SLP_S5, 0);
			ap_set_signal(dev, AP_OUT_SLP_S4, 0);
			ap_set_signal(dev, AP_OUT_SLP_S3, 0);
			ap_set_signal(dev, AP_OUT_SLP_S0, 0);
		}
		ap_set_signal(dev, AP_OUT_RSMRST_PWRGD, en_s5 ? 1 : 0);
		return;
	}

	/* Assert RSMRST_PWRGD once the EC powers up the S5 rails. */
	ap_set_signal(dev, AP_OUT_RSMRST_PWRGD, 1);

	switch (data->current_state) {
	case AP_STATE_G3:
	case AP_STATE_S5:
		/* Transition G3 -> S4 by deasserting SLP_S5# and SLP_S4# first.
		 * Hold in S4 briefly so the EC transitions S5 -> S4 -> S3
		 * before SLP_S3# deasserts and ALL_SYS_PWRGD asserts.
		 */
		LOG_INF("AP SM: G3 -> S4");
		data->current_state = AP_STATE_S4;
		data->s4_entry_ms = k_uptime_get();
		ap_set_signal(dev, AP_OUT_SLP_S5, 1);
		ap_set_signal(dev, AP_OUT_SLP_S4, 1);
		k_work_reschedule(&data->ap_poll_work, K_MSEC(15));
		return;

	case AP_STATE_S4:
		if ((k_uptime_get() - data->s4_entry_ms) < 15) {
			k_work_reschedule(&data->ap_poll_work, K_MSEC(5));
			return;
		}
		LOG_INF("AP SM: S4 -> S3");
		data->current_state = AP_STATE_S3;
		ap_set_signal(dev, AP_OUT_SLP_S3, 1);
		ap_set_signal(dev, AP_OUT_ALL_SYS_PWRGD, 1);
		/* If pch_pwrok and sys_pwrok are already high, proceed to S0 */
		if (pch_pwrok && sys_pwrok) {
			LOG_INF("AP SM: pch_pwrok & sys_pwrok high -> S0");
			data->current_state = AP_STATE_S0;
			ap_set_signal(dev, AP_OUT_SLP_S0, 1);
		}
		break;

	case AP_STATE_S3:
		ap_set_signal(dev, AP_OUT_ALL_SYS_PWRGD, 1);
		if (pch_pwrok && sys_pwrok) {
			LOG_INF("AP SM: S3 -> S0 "
				"(pch_pwrok & sys_pwrok asserted)");
			data->current_state = AP_STATE_S0;
			ap_set_signal(dev, AP_OUT_SLP_S0, 1);
		}
		break;

	case AP_STATE_S0:
		if (!pch_pwrok || !sys_pwrok) {
			LOG_INF("AP SM: pwrok deasserted -> S3");
			data->current_state = AP_STATE_S3;
			ap_set_signal(dev, AP_OUT_SLP_S0, 0);
		}
		break;
	}

	k_work_reschedule(&data->ap_poll_work,
			  (data->current_state == AP_STATE_S3 ||
			   pch_pwrok != sys_pwrok) ?
				  K_USEC(250) :
				  K_MSEC(20));
}

static uint16_t ap_get_signals_bitmask(const struct device *dev)
{
	const struct x86_mtl_emulator_config *cfg = dev->config;
	uint16_t sig = 0;

	if (gpio_pin_get_dt(&cfg->en_s5))
		sig |= AP_SIG_EN_S5;
	if (gpio_pin_get_dt(&cfg->rsmrst_n))
		sig |= AP_SIG_RSMRST;
	if (gpio_pin_get_dt(&cfg->pwrbtn_n))
		sig |= AP_SIG_SOC_PWRBTN;
	if (gpio_pin_get_dt(&cfg->sys_pwrok))
		sig |= AP_SIG_SYS_PWROK;
	if (gpio_pin_get_dt(&cfg->plt_pwrok))
		sig |= AP_SIG_PCH_PWROK;

	if (gpio_pin_get_dt(&cfg->rsmrst_pwrgd))
		sig |= AP_SIG_RSMRST_PWRGD;
	if (gpio_pin_get_dt(&cfg->all_sys_pwrgd))
		sig |= AP_SIG_ALL_SYS_PWRGD;
	if (gpio_pin_get_dt(&cfg->slp_s5))
		sig |= AP_SIG_SLP_S5;
	if (gpio_pin_get_dt(&cfg->slp_s4))
		sig |= AP_SIG_SLP_S4;
	if (gpio_pin_get_dt(&cfg->slp_s3))
		sig |= AP_SIG_SLP_S3;
	if (gpio_pin_get_dt(&cfg->slp_s0))
		sig |= AP_SIG_SLP_S0;
	if (ec_pwrbtn_get_state())
		sig |= AP_SIG_EC_PWRBTN;
	if (ec_acok_get_state())
		sig |= AP_SIG_ACOK;

	return sig;
}

static const char *state_to_str(enum ap_state state)
{
	switch (state) {
	case AP_STATE_G3:
		return "G3";
	case AP_STATE_S5:
		return "S5";
	case AP_STATE_S4:
		return "S4";
	case AP_STATE_S3:
		return "S3";
	case AP_STATE_S0:
		return "S0";
	default:
		return "UNKNOWN";
	}
}

static int cmd_ap_status(const struct shell *sh, size_t argc, char **argv)
{
	const struct x86_mtl_emulator_config *cfg = emul_dev->config;
	struct x86_mtl_emulator_data *data = emul_dev->data;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "AP Mode: %s",
		    data->current_mode == AP_MODE_AUTO ? "AUTO" : "MANUAL");
	shell_print(sh, "AP State: %s (%d)", state_to_str(data->current_state),
		    data->current_state);
	shell_print(sh, "Platform inputs from EC:");
	shell_print(sh, "  EN_PP5000_S5:    %d", gpio_pin_get_dt(&cfg->en_s5));
	shell_print(sh, "Platform outputs to EC:");
	shell_print(sh, "  ALL_SYS_PWRGD:   %d",
		    gpio_pin_get_dt(&cfg->all_sys_pwrgd));
	shell_print(sh, "  RSMRST_PWRGD_N:  %d",
		    gpio_pin_get_dt(&cfg->rsmrst_pwrgd));
	shell_print(sh, "MTL inputs from EC:");
	shell_print(sh, "  RSMRST#:         %d",
		    gpio_pin_get_dt(&cfg->rsmrst_n));
	shell_print(sh, "  PWRBTN#:         %d",
		    gpio_pin_get_dt(&cfg->pwrbtn_n));
	shell_print(sh, "  PTL_PWROK:       %d",
		    gpio_pin_get_dt(&cfg->plt_pwrok));
	shell_print(sh, "  SYS_PWROK:       %d",
		    gpio_pin_get_dt(&cfg->sys_pwrok));
	shell_print(sh, "MTL outputs to EC:");
	shell_print(sh, "  SLP_S5_L:        %d", gpio_pin_get_dt(&cfg->slp_s5));
	shell_print(sh, "  SLP_S4_L:        %d", gpio_pin_get_dt(&cfg->slp_s4));
	shell_print(sh, "  SLP_S3_L:        %d", gpio_pin_get_dt(&cfg->slp_s3));
	shell_print(sh, "  SLP_S0_L:        %d", gpio_pin_get_dt(&cfg->slp_s0));
	shell_print(sh, "Stimulus outputs to EC:");
	shell_print(sh, "  EC_PWRBTN_L:     %d", ec_pwrbtn_get_state());
	shell_print(sh, "  ACOK:            %d", ec_acok_get_state());

	return 0;
}

static int cmd_ap_timings(const struct shell *sh, size_t argc, char **argv)
{
	struct x86_mtl_emulator_data *data = emul_dev->data;
	const struct usb_ap_timings *t = &data->timings;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "AP Power Sequence Timings:");
	shell_print(sh, "  tPCH03 (RSMRST_PWRGD -> RSMRST# deassert): %u us",
		    t->rsmrst_pwrgd_to_rsmrst_deassert_us);
	shell_print(sh, "  tPCH08 (SLP_S3# deassert -> PLT_PWROK):    %u us",
		    t->slp_s3_to_plt_pwrok_us);
	shell_print(sh, "  tPLT04 (ALL_SYS_PWRGD -> PLT_PWROK):       %u us",
		    t->all_sys_pwrgd_to_plt_pwrok_us);
	shell_print(sh, "  tPLT05 (ALL_SYS_PWRGD -> SYS_PWROK):       %u us",
		    t->all_sys_pwrgd_to_sys_pwrok_us);
	shell_print(sh, "  PWROK delta (PLT_PWROK -> SYS_PWROK):      %u us",
		    t->plt_pwrok_to_sys_pwrok_us);
	shell_print(sh, "  tCPU22 (SLP_S3# assert -> PLT_PWROK low):  %u us",
		    t->slp_s3_assert_to_plt_pwrok_deassert_us);
	shell_print(sh, "  tPCH12 (RSMRST# assert -> EN_S5 low):      %u ns",
		    t->rsmrst_assert_to_en_s5_deassert_ns);
	shell_print(sh, "  tPCH43 (PWRBTN# pulse width):              %u us",
		    t->pwrbtn_pulse_width_us);

	return 0;
}

static int cmd_ap_mode(const struct shell *sh, size_t argc, char **argv)
{
	struct x86_mtl_emulator_data *data = emul_dev->data;

	if (argc < 2) {
		shell_print(sh, "Mode: %s",
			    data->current_mode == AP_MODE_AUTO ? "auto" :
								 "manual");
		return 0;
	}

	if (strcmp(argv[1], "auto") == 0) {
		data->current_mode = AP_MODE_AUTO;
		shell_print(sh, "Mode set to AUTO");
		ap_update_state_machine(emul_dev);
	} else if (strcmp(argv[1], "manual") == 0) {
		data->current_mode = AP_MODE_MANUAL;
		shell_print(sh, "Mode set to MANUAL");
	} else {
		shell_error(sh, "Usage: ap mode [auto|manual]");
		return -EINVAL;
	}
	return 0;
}

static int cmd_ap_set(const struct shell *sh, size_t argc, char **argv)
{
	static const struct {
		const char *name;
		enum ap_output_signal sig;
	} ap_signals[] = {
		{ "rsmrst_pwrgd", AP_OUT_RSMRST_PWRGD },
		{ "all_sys_pwrgd", AP_OUT_ALL_SYS_PWRGD },
		{ "slp_s5", AP_OUT_SLP_S5 },
		{ "slp_s4", AP_OUT_SLP_S4 },
		{ "slp_s3", AP_OUT_SLP_S3 },
		{ "slp_s0", AP_OUT_SLP_S0 },
	};

	if (argc < 3) {
		shell_error(sh, "Usage: ap set <signal> <0|1>");
		shell_print(sh, "Signals: rsmrst_pwrgd, all_sys_pwrgd, slp_s5, "
				"slp_s4, slp_s3, slp_s0");
		return -EINVAL;
	}

	const char *sig = argv[1];
	int val = atoi(argv[2]);

	for (size_t i = 0; i < ARRAY_SIZE(ap_signals); i++) {
		if (strcmp(sig, ap_signals[i].name) == 0) {
			ap_set_signal(emul_dev, ap_signals[i].sig, val);
			shell_print(sh, "Set %s to %d", sig, val);
			return 0;
		}
	}

	shell_error(sh, "Unknown signal: %s", sig);
	return -EINVAL;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	ap_cmds,
	SHELL_CMD_ARG(status, NULL, "Show AP status and signals", cmd_ap_status,
		      1, 0),
	SHELL_CMD_ARG(timings, NULL, "Show AP power sequence timings",
		      cmd_ap_timings, 1, 0),
	SHELL_CMD_ARG(mode, NULL, "Set/get AP mode: ap mode [auto|manual]",
		      cmd_ap_mode, 1, 1),
	SHELL_CMD_ARG(set, NULL, "Set output signal: ap set <signal> <0|1>",
		      cmd_ap_set, 3, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(ap, &ap_cmds, "AP emulator commands", NULL);

static void x86_mtl_emulator_to_dev(uint16_t index, uint16_t value,
				    const uint8_t *buf, size_t size)
{
	struct x86_mtl_emulator_data *data = emul_dev->data;

	ARG_UNUSED(buf);
	ARG_UNUSED(size);

	switch (index) {
	case USB_REQ_AP_SET_MODE:
		data->current_mode = (value != 0) ? AP_MODE_MANUAL :
						    AP_MODE_AUTO;
		if (data->current_mode == AP_MODE_AUTO) {
			ap_update_state_machine(emul_dev);
		}
		break;
	case USB_REQ_AP_SET_SIGNAL: {
		enum ap_output_signal sig_id = (value >> 8) & 0xFF;
		uint8_t sig_val = value & 0xFF;

		ap_set_signal(emul_dev, sig_id, sig_val);
		k_work_submit(&data->ap_work);
		break;
	}
	default:
		break;
	}
}

static int x86_mtl_emulator_to_host(uint16_t index, uint16_t value,
				    uint8_t *buf, size_t size)
{
	struct x86_mtl_emulator_data *data = emul_dev->data;

	ARG_UNUSED(value);

	if (index == USB_REQ_AP_GET_STATUS) {
		if (size < sizeof(struct usb_ap_status)) {
			return -ENOMEM;
		}

		struct usb_ap_status *status = (struct usb_ap_status *)buf;

		status->state = data->current_state;
		status->mode = data->current_mode;
		status->signals = ap_get_signals_bitmask(emul_dev);

		return sizeof(struct usb_ap_status);
	} else if (index == USB_REQ_AP_GET_TIMINGS) {
		if (size < sizeof(struct usb_ap_timings)) {
			return -ENOMEM;
		}

		memcpy(buf, &data->timings, sizeof(struct usb_ap_timings));
		return sizeof(struct usb_ap_timings);
	}

	return 0;
}

USB_REQUEST_CALLBACK_DEFINE(x86_mtl_emulator_to_dev, x86_mtl_emulator_to_host);

static int configure_input_irq(const struct gpio_dt_spec *spec,
			       struct gpio_callback *cb,
			       gpio_callback_handler_t handler)
{
	int ret;

	ret = gpio_pin_configure_dt(spec, GPIO_INPUT);
	if (ret < 0) {
		return ret;
	}

	gpio_init_callback(cb, handler, BIT(spec->pin));
	ret = gpio_add_callback(spec->port, cb);
	if (ret < 0) {
		return ret;
	}

	return gpio_pin_interrupt_configure_dt(spec, GPIO_INT_EDGE_BOTH);
}

static int x86_mtl_emulator_init(const struct device *dev)
{
	const struct x86_mtl_emulator_config *cfg = dev->config;
	struct x86_mtl_emulator_data *data = dev->data;

	const struct gpio_dt_spec *const gpios[] = {
		&cfg->en_s5,	     &cfg->rsmrst_n,  &cfg->pwrbtn_n,
		&cfg->sys_pwrok,     &cfg->plt_pwrok, &cfg->rsmrst_pwrgd,
		&cfg->all_sys_pwrgd, &cfg->slp_s5,    &cfg->slp_s4,
		&cfg->slp_s3,	     &cfg->slp_s0,
	};

	for (size_t i = 0; i < ARRAY_SIZE(gpios); i++) {
		if (!gpio_is_ready_dt(gpios[i])) {
			LOG_ERR_DEVICE_NOT_READY(gpios[i]->port);
			return -ENODEV;
		}
	}

	data->dev = dev;

	k_work_init(&data->ap_work, ap_work_handler);
	k_work_init_delayable(&data->ap_poll_work, ap_poll_work_handler);

	/* Configure outputs */
	gpio_pin_configure_dt(&cfg->rsmrst_pwrgd, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&cfg->all_sys_pwrgd, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&cfg->slp_s5, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&cfg->slp_s4, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&cfg->slp_s3, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&cfg->slp_s0, GPIO_OUTPUT_INACTIVE);

	/* Configure inputs with interrupts */
	configure_input_irq(&cfg->en_s5, &data->cb_en_s5, en_s5_callback);
	configure_input_irq(&cfg->rsmrst_n, &data->cb_rsmrst_n,
			    rsmrst_n_callback);
	configure_input_irq(&cfg->pwrbtn_n, &data->cb_pwrbtn_n,
			    pwrbtn_n_callback);
	configure_input_irq(&cfg->plt_pwrok, &data->cb_plt_pwrok,
			    plt_pwrok_callback);

	/* Note: PI01 shares EXTI1 with PB01 (user_button). Polled in
	 * ap_poll_work. */
	gpio_pin_configure_dt(&cfg->sys_pwrok, GPIO_INPUT);

	data->prev_en_s5 = gpio_pin_get_dt(&cfg->en_s5);
	data->prev_rsmrst = gpio_pin_get_dt(&cfg->rsmrst_n);
	data->prev_pwrbtn = gpio_pin_get_dt(&cfg->pwrbtn_n);
	data->prev_plt_pwrok = gpio_pin_get_dt(&cfg->plt_pwrok);
	data->prev_sys_pwrok = gpio_pin_get_dt(&cfg->sys_pwrok);

	/* Initial evaluation */
	ap_update_state_machine(dev);

	LOG_INF("x86 MTL AP emulator initialized");
	return 0;
}

#define X86_MTL_EMULATOR_INIT(n)                                              \
	static const struct x86_mtl_emulator_config mtl_emul_cfg_##n = {      \
		.en_s5 = GPIO_DT_SPEC_INST_GET(n, en_s5_gpios),               \
		.rsmrst_n = GPIO_DT_SPEC_INST_GET(n, rsmrst_n_gpios),         \
		.pwrbtn_n = GPIO_DT_SPEC_INST_GET(n, pwrbtn_n_gpios),         \
		.sys_pwrok = GPIO_DT_SPEC_INST_GET(n, sys_pwrok_gpios),       \
		.plt_pwrok = GPIO_DT_SPEC_INST_GET(n, plt_pwrok_gpios),       \
		.rsmrst_pwrgd = GPIO_DT_SPEC_INST_GET(n, rsmrst_pwrgd_gpios), \
		.all_sys_pwrgd =                                              \
			GPIO_DT_SPEC_INST_GET(n, all_sys_pwrgd_gpios),        \
		.slp_s5 = GPIO_DT_SPEC_INST_GET(n, slp_s5_gpios),             \
		.slp_s4 = GPIO_DT_SPEC_INST_GET(n, slp_s4_gpios),             \
		.slp_s3 = GPIO_DT_SPEC_INST_GET(n, slp_s3_gpios),             \
		.slp_s0 = GPIO_DT_SPEC_INST_GET(n, slp_s0_gpios),             \
	};                                                                    \
	static struct x86_mtl_emulator_data mtl_emul_data_##n = {             \
		.current_mode = AP_MODE_AUTO,                                 \
		.current_state = AP_STATE_G3,                                 \
	};                                                                    \
	DEVICE_DT_INST_DEFINE(n, x86_mtl_emulator_init, NULL,                 \
			      &mtl_emul_data_##n, &mtl_emul_cfg_##n,          \
			      POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY,  \
			      NULL);

DT_INST_FOREACH_STATUS_OKAY(X86_MTL_EMULATOR_INIT)
