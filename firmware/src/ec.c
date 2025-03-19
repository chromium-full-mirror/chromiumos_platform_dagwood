/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define DT_DRV_COMPAT cros_aic_pins

#include "ec.h"
#include "usb_request.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(ec, LOG_LEVEL_INF);

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
	     "only one 'cros,aic-pins' compatible node may be present");

static const struct gpio_dt_spec ec_rst_gpio =
	GPIO_DT_SPEC_INST_GET(0, ec_rst_gpios);

static const struct gpio_dt_spec ec_io_gpio[] = { DT_INST_FOREACH_PROP_ELEM_SEP(
	0, io_gpios, GPIO_DT_SPEC_GET_BY_IDX, (, )) };

static const struct device *reg_pp3300_mecc_core =
	DEVICE_DT_GET(DT_NODELABEL(pp3300_mecc_core));
static const struct device *reg_pp3300_mecc_z =
	DEVICE_DT_GET(DT_NODELABEL(pp3300_mecc_z));
static const struct device *reg_pp3300_mecc_s =
	DEVICE_DT_GET(DT_NODELABEL(pp3300_mecc_s));
static const struct device *reg_ppvar_mecc_vref =
	DEVICE_DT_GET(DT_NODELABEL(ppvar_mecc_vref));
static const struct device *reg_pp1800_mecc_z =
	DEVICE_DT_GET(DT_NODELABEL(pp1800_mecc_z));
static const struct device *reg_pp1800_mecc_s =
	DEVICE_DT_GET(DT_NODELABEL(pp1800_mecc_s));
static const struct device *reg_pp5000_mecc_z =
	DEVICE_DT_GET(DT_NODELABEL(pp5000_mecc_z));
static const struct device *reg_ppvar_mecc_rtc =
	DEVICE_DT_GET(DT_NODELABEL(ppvar_mecc_rtc));

static bool ec_power_enabled;

static void ec_power(bool enable)
{
	if (ec_power_enabled == enable) {
		return;
	}

	if (enable) {
		regulator_enable(reg_pp3300_mecc_core);
		regulator_enable(reg_pp3300_mecc_z);
		regulator_enable(reg_pp3300_mecc_s);
		regulator_enable(reg_ppvar_mecc_vref);
		regulator_enable(reg_pp1800_mecc_z);
		regulator_enable(reg_pp1800_mecc_s);
		regulator_enable(reg_pp5000_mecc_z);
		regulator_enable(reg_ppvar_mecc_rtc);
	} else {
		regulator_disable(reg_ppvar_mecc_rtc);
		regulator_disable(reg_pp5000_mecc_z);
		regulator_disable(reg_pp1800_mecc_s);
		regulator_disable(reg_pp1800_mecc_z);
		regulator_disable(reg_ppvar_mecc_vref);
		regulator_disable(reg_pp3300_mecc_s);
		regulator_disable(reg_pp3300_mecc_z);
		regulator_disable(reg_pp3300_mecc_core);
	}

	ec_power_enabled = enable;
}

static void button_input_cb(struct input_event *evt, void *user_data)
{
	if (evt->type != INPUT_EV_KEY) {
		return;
	}

	if (!evt->value) {
		return;
	}

	switch (evt->code) {
	case INPUT_KEY_0:
		ec_power(!ec_power_enabled);
		break;
	default:
		LOG_WRN("unknown code: %d", evt->code);
	}
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(gpio_keys)), button_input_cb,
		      NULL);

static int expect_pin(const struct shell *sh, uint8_t i)
{
	/* TODO: make it work with more than 32 pins */
	uint32_t state = 0;
	uint32_t expect = BIT(i);

	for (uint8_t j = 0; j < ARRAY_SIZE(ec_io_gpio); j++) {
		const struct gpio_dt_spec *gpio = &ec_io_gpio[j];
		int val;

		val = gpio_pin_get_dt(gpio);
		if (val) {
			state |= BIT(j);
		}
	}

	if (state != expect) {
		shell_print(sh, "unexpected state (%d) %08x != %08x", i, state,
			    expect);
		return -1;
	}

	shell_info(sh, "match state (%d) %08x", i, state);
	return 0;
}

#define IO_SWEEP_TIMEOUT_MS 5000

static int cmd_io_sweep(const struct shell *sh, size_t argc, char **argv)
{
	uint64_t start_time;
	int ret;

	for (uint8_t i = 0; i < ARRAY_SIZE(ec_io_gpio); i++) {
		const struct gpio_dt_spec *gpio = &ec_io_gpio[i];

		ret = gpio_pin_configure_dt(gpio, GPIO_INPUT);
		if (ret < 0) {
			LOG_ERR("gpio configuration failed: %d", ret);
			return ret;
		}
	}

	start_time = k_uptime_get();
	for (uint8_t i = 0; i < ARRAY_SIZE(ec_io_gpio); i++) {
		while (true) {
			if (k_uptime_get() - start_time > IO_SWEEP_TIMEOUT_MS) {
				goto timeout;
			}

			ret = expect_pin(sh, i);
			if (ret < 0) {
				k_sleep(K_MSEC(100));
				continue;
			}
			break;
		}
	}

	shell_info(sh, "sweep matched all pins successfully");

	return 0;

timeout:
	shell_error(sh, "timeout");

	return -ETIME;
}

void ec_reset(void)
{
	gpio_pin_set_dt(&ec_rst_gpio, 1);

	k_sleep(K_MSEC(100));

	gpio_pin_set_dt(&ec_rst_gpio, 0);
}

static int cmd_ec_reset(const struct shell *sh, size_t argc, char **argv)
{
	ec_reset();

	return 0;
}

static int cmd_ec_power(const struct shell *sh, size_t argc, char **argv)
{
	int err = 0;
	bool on;

	on = shell_strtobool(argv[1], 0, &err);
	if (err) {
		shell_error(sh, "Invalid argument: %s", argv[2]);
		return err;
	}

	ec_power(on);

	return 0;
}

/* clang-format off */
SHELL_STATIC_SUBCMD_SET_CREATE(ec_cmds,
	SHELL_CMD_ARG(io_sweep, NULL, "io_sweep", cmd_io_sweep, 1, 0),
	SHELL_CMD_ARG(power, NULL, "power on|off", cmd_ec_power, 2, 0),
	SHELL_CMD_ARG(reset, NULL, "reset", cmd_ec_reset, 1, 0),
	SHELL_SUBCMD_SET_END);
/* clang-format on */

SHELL_CMD_REGISTER(ec, &ec_cmds, "EC control commands", NULL);

static void ec_cb(uint16_t index, uint16_t value)
{
	switch (index) {
	case USB_REQ_EC_RESET:
		LOG_INF("usb ec reset");
		ec_reset();
		break;
	case USB_REQ_POWER:
		LOG_INF("usb ec power %d", value);
		ec_power(value);
		break;
	}
}

USB_REQUEST_CALLBACK_DEFINE(ec_cb);

static int ec_init(void)
{
	int ret;

	ret = gpio_pin_configure_dt(&ec_rst_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("gpio configuration failed: %d", ret);
		return ret;
	}

	return 0;
}

SYS_INIT(ec_init, APPLICATION, 99);
