/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "usb_request.h"

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(fake_fan, LOG_LEVEL_INF);

#define DT_DRV_COMPAT cros_fake_fan

static const struct pwm_dt_spec pwm_in = PWM_DT_SPEC_INST_GET_BY_IDX(0, 0);
static const struct pwm_dt_spec tach_out = PWM_DT_SPEC_INST_GET_BY_IDX(0, 1);

static int fake_fan_get_pwm(uint32_t *freq, uint32_t *dc)
{
	uint64_t period_ns;
	uint64_t width_ns;
	int ret;

	ret = pwm_capture_nsec(pwm_in.dev, pwm_in.channel,
			       PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE,
			       &period_ns, &width_ns, K_MSEC(500));
	if (ret < 0) {
		return ret;
	}

	if (period_ns == 0) {
		return -EINVAL;
	}

	*freq = NSEC_PER_SEC / period_ns;
	*dc = DIV_ROUND_CLOSEST(width_ns * 100, period_ns);

	return 0;
}

static int fake_fan_set_tach(uint32_t rpm)
{
	LOG_INF("set tach: %d rpm", rpm);

	if (rpm == 0) {
		return pwm_set_dt(&tach_out, 0, 0);
	}

	uint32_t period = (uint64_t)NSEC_PER_SEC / rpm * 60 / 2;
	return pwm_set_dt(&tach_out, period, period / 2);
}

static int cmd_get_pwm(const struct shell *sh, size_t argc, char **argv)
{
	uint32_t freq;
	uint32_t dc;
	int ret;

	ret = fake_fan_get_pwm(&freq, &dc);
	if (ret < 0) {
		shell_error(sh, "pwm_capture_nsec error, ret: %d", ret);
		return ret;
	}

	shell_print(sh, "fan pwm: %d Hz %d %%", freq, dc);

	return 0;
}

static int cmd_set_tach(const struct shell *sh, size_t argc, char **argv)
{
	int err = 0;
	int rpm;

	rpm = shell_strtoul(argv[1], 0, &err);
	if (err) {
		shell_error(sh, "Invalid argument: %s", argv[1]);
		return err;
	}

	return fake_fan_set_tach(rpm);
}

/* clang-format off */
SHELL_STATIC_SUBCMD_SET_CREATE(fan_cmds,
        SHELL_CMD_ARG(get_pwm, NULL, "get_pwm", cmd_get_pwm, 1, 0),
        SHELL_CMD_ARG(set_tach, NULL, SHELL_HELP("set_tach", "rpm"),
		      cmd_set_tach, 2, 0),
        SHELL_SUBCMD_SET_END);
/* clang-format on */

SHELL_CMD_REGISTER(fan, &fan_cmds, "Fan commands", NULL);

static void fake_fan_to_dev(uint16_t index, uint16_t value, const uint8_t *buf,
			    size_t size)
{
	if (index != USB_REQ_FAN_SET_TACH) {
		return;
	}

	fake_fan_set_tach(value);
}

static int fake_fan_to_host(uint16_t index, uint16_t value, uint8_t *buf,
			    size_t size)
{
	struct usb_fan_pwm *data = (struct usb_fan_pwm *)buf;
	int ret;

	if (index != USB_REQ_FAN_GET_PWM) {
		return 0;
	}

	if (size < sizeof(*data)) {
		return -ENOMEM;
	}

	ret = fake_fan_get_pwm(&data->frequency, &data->duty_cycle);
	if (ret < 0) {
		return ret;
	}

	return sizeof(*data);
}

USB_REQUEST_CALLBACK_DEFINE(fake_fan_to_dev, fake_fan_to_host);
