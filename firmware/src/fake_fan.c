/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

static int cmd_get_pwm(const struct shell *sh, size_t argc, char **argv)
{
	uint64_t period_ns;
	uint64_t width_ns;
	int ret;

	ret = pwm_capture_nsec(pwm_in.dev, pwm_in.channel,
			       PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE,
			       &period_ns, &width_ns, K_MSEC(500));
	if (ret < 0) {
		shell_error(sh, "pwm_capture_nsec error, ret: %d", ret);
		return ret;
	}

	if (period_ns == 0) {
		return -EINVAL;
	}

	uint32_t freq = NSEC_PER_SEC / period_ns;
	uint32_t dc = DIV_ROUND_CLOSEST(width_ns * 100, period_ns);

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

	if (rpm == 0) {
		pwm_set_dt(&tach_out, 0, 0);
		return 0;
	}

	uint32_t period = (uint64_t)NSEC_PER_SEC / rpm * 60 / 2;
	pwm_set_dt(&tach_out, period, period / 2);

	return 0;
}

/* clang-format off */
SHELL_STATIC_SUBCMD_SET_CREATE(fan_cmds,
        SHELL_CMD_ARG(get_pwm, NULL, "get_pwm", cmd_get_pwm, 1, 0),
        SHELL_CMD_ARG(set_tach, NULL, SHELL_HELP("set_tach", "rpm"),
		      cmd_set_tach, 2, 0),
        SHELL_SUBCMD_SET_END);
/* clang-format on */

SHELL_CMD_REGISTER(fan, &fan_cmds, "Fan commands", NULL);
