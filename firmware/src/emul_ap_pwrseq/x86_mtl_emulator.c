/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define DT_DRV_COMPAT cros_x86_mtl_emulator

#include "../usb_request.h"
#include "ec_acok.h"
#include "ec_pwrbtn.h"

#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
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
};

static const struct device *const emul_dev = DEVICE_DT_INST_GET(0);

static void ap_update_state_machine(const struct device *dev);

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

static void gpio_event_callback(const struct device *port,
				struct gpio_callback *cb, uint32_t pins)
{
	struct x86_mtl_emulator_data *data = emul_dev->data;

	ARG_UNUSED(port);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	k_work_submit(&data->ap_work);
}

static void ap_update_state_machine(const struct device *dev)
{
	const struct x86_mtl_emulator_config *cfg = dev->config;
	struct x86_mtl_emulator_data *data = dev->data;

	if (data->current_mode != AP_MODE_AUTO) {
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
		/* EN_S5 deasserted or RSMRST asserted -> reset back to G3 or S5
		 * unpowered */
		if (data->current_state != AP_STATE_G3) {
			LOG_INF("AP SM: RSMRST asserted or EN_S5 deasserted -> "
				"G3");
			data->current_state = AP_STATE_G3;
			gpio_pin_set_dt(&cfg->all_sys_pwrgd, 0);
			gpio_pin_set_dt(&cfg->slp_s5, 0);
			gpio_pin_set_dt(&cfg->slp_s4, 0);
			gpio_pin_set_dt(&cfg->slp_s3, 0);
			gpio_pin_set_dt(&cfg->slp_s0, 0);
		}
		gpio_pin_set_dt(&cfg->rsmrst_pwrgd, en_s5 ? 1 : 0);
		if (en_s5) {
			k_work_reschedule(&data->ap_poll_work, K_MSEC(20));
		}
		return;
	}

	/* Assert RSMRST_PWRGD once the EC powers up the S5 rails. */
	gpio_pin_set_dt(&cfg->rsmrst_pwrgd, 1);

	switch (data->current_state) {
	case AP_STATE_G3:
		/* Transition G3 -> S5 -> S4 -> S3.
		 * Actual hardware doesn't have a minimum delay between
		 * SLP_S5#, SLP_S4#, and SLP_S3# deassertion so we transition
		 * directly from G3 to S3.
		 */
		LOG_INF("AP SM: G3 -> S3");
		data->current_state = AP_STATE_S3;
		gpio_pin_set_dt(&cfg->slp_s5, 1);
		gpio_pin_set_dt(&cfg->slp_s4, 1);
		gpio_pin_set_dt(&cfg->slp_s3, 1);
		gpio_pin_set_dt(&cfg->all_sys_pwrgd, 1);
		/* If pch_pwrok and sys_pwrok are already high, proceed to S0 */
		if (pch_pwrok && sys_pwrok) {
			LOG_INF("AP SM: pch_pwrok & sys_pwrok high -> S0");
			data->current_state = AP_STATE_S0;
			gpio_pin_set_dt(&cfg->slp_s0, 1);
		}
		break;

	case AP_STATE_S5:
	case AP_STATE_S4:
	case AP_STATE_S3:
		gpio_pin_set_dt(&cfg->all_sys_pwrgd, 1);
		if (pch_pwrok && sys_pwrok) {
			LOG_INF("AP SM: S3 -> S0 "
				"(pch_pwrok & sys_pwrok asserted)");
			data->current_state = AP_STATE_S0;
			gpio_pin_set_dt(&cfg->slp_s0, 1);
		}
		break;

	case AP_STATE_S0:
		if (!pch_pwrok || !sys_pwrok) {
			LOG_INF("AP SM: pwrok deasserted -> S3");
			data->current_state = AP_STATE_S3;
			gpio_pin_set_dt(&cfg->slp_s0, 0);
		}
		break;
	}

	k_work_reschedule(&data->ap_poll_work, K_MSEC(20));
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
	const struct x86_mtl_emulator_config *cfg = emul_dev->config;

	if (argc < 3) {
		shell_error(sh, "Usage: ap set <signal> <0|1>");
		shell_print(sh, "Signals: rsmrst_pwrgd, all_sys_pwrgd, slp_s5, "
				"slp_s4, slp_s3, slp_s0");
		return -EINVAL;
	}

	const char *sig = argv[1];
	int val = atoi(argv[2]);

	if (strcmp(sig, "rsmrst_pwrgd") == 0) {
		gpio_pin_set_dt(&cfg->rsmrst_pwrgd, val);
	} else if (strcmp(sig, "all_sys_pwrgd") == 0) {
		gpio_pin_set_dt(&cfg->all_sys_pwrgd, val);
	} else if (strcmp(sig, "slp_s5") == 0) {
		gpio_pin_set_dt(&cfg->slp_s5, val);
	} else if (strcmp(sig, "slp_s4") == 0) {
		gpio_pin_set_dt(&cfg->slp_s4, val);
	} else if (strcmp(sig, "slp_s3") == 0) {
		gpio_pin_set_dt(&cfg->slp_s3, val);
	} else if (strcmp(sig, "slp_s0") == 0) {
		gpio_pin_set_dt(&cfg->slp_s0, val);
	} else {
		shell_error(sh, "Unknown signal: %s", sig);
		return -EINVAL;
	}

	shell_print(sh, "Set %s to %d", sig, val);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	ap_cmds,
	SHELL_CMD_ARG(status, NULL, "Show AP status and signals", cmd_ap_status,
		      1, 0),
	SHELL_CMD_ARG(mode, NULL, "Set/get AP mode: ap mode [auto|manual]",
		      cmd_ap_mode, 1, 1),
	SHELL_CMD_ARG(set, NULL, "Set output signal: ap set <signal> <0|1>",
		      cmd_ap_set, 3, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(ap, &ap_cmds, "AP emulator commands", NULL);

static void x86_mtl_emulator_to_dev(uint16_t index, uint16_t value,
				    const uint8_t *buf, size_t size)
{
	const struct x86_mtl_emulator_config *cfg = emul_dev->config;
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
		uint8_t sig_id = (value >> 8) & 0xFF;
		uint8_t sig_val = value & 0xFF;

		switch (sig_id) {
		case 0:
			gpio_pin_set_dt(&cfg->rsmrst_pwrgd, sig_val);
			break;
		case 1:
			gpio_pin_set_dt(&cfg->all_sys_pwrgd, sig_val);
			break;
		case 2:
			gpio_pin_set_dt(&cfg->slp_s5, sig_val);
			break;
		case 3:
			gpio_pin_set_dt(&cfg->slp_s4, sig_val);
			break;
		case 4:
			gpio_pin_set_dt(&cfg->slp_s3, sig_val);
			break;
		case 5:
			gpio_pin_set_dt(&cfg->slp_s0, sig_val);
			break;
		}
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

	if (index != USB_REQ_AP_GET_STATUS) {
		return 0;
	}

	if (size < sizeof(struct usb_ap_status)) {
		return -ENOMEM;
	}

	struct usb_ap_status *status = (struct usb_ap_status *)buf;

	status->state = data->current_state;
	status->mode = data->current_mode;
	status->signals = ap_get_signals_bitmask(emul_dev);

	return sizeof(struct usb_ap_status);
}

USB_REQUEST_CALLBACK_DEFINE(x86_mtl_emulator_to_dev, x86_mtl_emulator_to_host);

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
	gpio_pin_configure_dt(&cfg->en_s5, GPIO_INPUT);
	gpio_init_callback(&data->cb_en_s5, gpio_event_callback,
			   BIT(cfg->en_s5.pin));
	gpio_add_callback(cfg->en_s5.port, &data->cb_en_s5);
	gpio_pin_interrupt_configure_dt(&cfg->en_s5, GPIO_INT_EDGE_BOTH);

	gpio_pin_configure_dt(&cfg->rsmrst_n, GPIO_INPUT);
	gpio_init_callback(&data->cb_rsmrst_n, gpio_event_callback,
			   BIT(cfg->rsmrst_n.pin));
	gpio_add_callback(cfg->rsmrst_n.port, &data->cb_rsmrst_n);
	gpio_pin_interrupt_configure_dt(&cfg->rsmrst_n, GPIO_INT_EDGE_BOTH);

	gpio_pin_configure_dt(&cfg->pwrbtn_n, GPIO_INPUT);
	gpio_init_callback(&data->cb_pwrbtn_n, gpio_event_callback,
			   BIT(cfg->pwrbtn_n.pin));
	gpio_add_callback(cfg->pwrbtn_n.port, &data->cb_pwrbtn_n);
	gpio_pin_interrupt_configure_dt(&cfg->pwrbtn_n, GPIO_INT_EDGE_BOTH);

	gpio_pin_configure_dt(&cfg->plt_pwrok, GPIO_INPUT);
	gpio_init_callback(&data->cb_plt_pwrok, gpio_event_callback,
			   BIT(cfg->plt_pwrok.pin));
	gpio_add_callback(cfg->plt_pwrok.port, &data->cb_plt_pwrok);
	gpio_pin_interrupt_configure_dt(&cfg->plt_pwrok, GPIO_INT_EDGE_BOTH);

	/* Note: PI01 shares EXTI1 with PB01 (user_button). Polled in
	 * ap_poll_work. */
	gpio_pin_configure_dt(&cfg->sys_pwrok, GPIO_INPUT);

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
