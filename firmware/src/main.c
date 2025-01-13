/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/drivers/led.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

const struct led_dt_spec status_led = LED_DT_SPEC_GET(DT_NODELABEL(status_led));

int main(void)
{
	LOG_INF("started");

	while (true) {
		led_on_dt(&status_led);
		k_sleep(K_MSEC(1000));
		led_off_dt(&status_led);
		k_sleep(K_MSEC(1000));
	}
}

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

static int cmd_ec_power(const struct shell *sh, size_t argc, char **argv)
{
	int err = 0;
	bool on;

	on = shell_strtobool(argv[1], 0, &err);
	if (err) {
		shell_error(sh, "Invalid argument: %s", argv[2]);
		return err;
	}

	if (on) {
		shell_info(sh, "EC power on");

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

		shell_info(sh, "EC power off");
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(ec_cmds,
			       SHELL_CMD_ARG(power, NULL, "power on|off",
					     cmd_ec_power, 2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(ec, &ec_cmds, "EC control commands", NULL);
