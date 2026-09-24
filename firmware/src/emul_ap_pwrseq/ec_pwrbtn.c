/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define DT_DRV_COMPAT cros_ec_pwrbtn

#include "../usb_request.h"
#include "ec_pwrbtn.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(ec_pwrbtn, LOG_LEVEL_INF);

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
	     "only one 'cros,ec-pwrbtn' compatible node may be present");

struct ec_pwrbtn_config {
	struct gpio_dt_spec ec_pwrbtn;
};

struct ec_pwrbtn_data {
	const struct device *dev;
	struct k_work_delayable release_work;
};

static const struct device *const pwrbtn_dev = DEVICE_DT_INST_GET(0);

static void ec_pwrbtn_release_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct ec_pwrbtn_data *data =
		CONTAINER_OF(dwork, struct ec_pwrbtn_data, release_work);
	const struct ec_pwrbtn_config *cfg = data->dev->config;

	gpio_pin_set_dt(&cfg->ec_pwrbtn, 0);
	LOG_INF("EC power button released");
}

int ec_pwrbtn_press(uint32_t duration_ms)
{
	const struct ec_pwrbtn_config *cfg = pwrbtn_dev->config;
	struct ec_pwrbtn_data *data = pwrbtn_dev->data;
	int ret;

	if (!device_is_ready(pwrbtn_dev)) {
		return -ENODEV;
	}

	if (duration_ms == 0) {
		duration_ms = 200;
	}

	ret = gpio_pin_set_dt(&cfg->ec_pwrbtn, 1);
	if (ret < 0) {
		return ret;
	}

	k_work_reschedule(&data->release_work, K_MSEC(duration_ms));
	return 0;
}

int ec_pwrbtn_get_state(void)
{
	const struct ec_pwrbtn_config *cfg = pwrbtn_dev->config;

	if (!device_is_ready(pwrbtn_dev)) {
		return 0;
	}

	return gpio_pin_get_dt(&cfg->ec_pwrbtn);
}

static int cmd_pwrbtn(const struct shell *sh, size_t argc, char **argv)
{
	uint32_t ms = 200;
	int ret;

	if (argc >= 2) {
		int err = 0;

		ms = shell_strtoul(argv[1], 0, &err);
		if (err || ms == 0) {
			shell_error(sh, "Invalid duration: %s", argv[1]);
			return -EINVAL;
		}
	}

	ret = ec_pwrbtn_press(ms);
	if (ret < 0) {
		shell_error(sh, "Failed to press EC power button: %d", ret);
		return ret;
	}

	shell_print(sh, "EC power button pressed for %u ms", ms);
	return 0;
}

SHELL_CMD_ARG_REGISTER(pwrbtn, NULL, "Press EC power button: pwrbtn [ms]",
		       cmd_pwrbtn, 1, 1);

static void ec_pwrbtn_to_dev(uint16_t index, uint16_t value, const uint8_t *buf,
			     size_t size)
{
	ARG_UNUSED(buf);
	ARG_UNUSED(size);

	if (index != USB_REQ_AP_PWRBTN) {
		return;
	}

	ec_pwrbtn_press(value);
}

USB_REQUEST_CALLBACK_DEFINE(ec_pwrbtn_to_dev, NULL);

static int ec_pwrbtn_init(const struct device *dev)
{
	const struct ec_pwrbtn_config *cfg = dev->config;
	struct ec_pwrbtn_data *data = dev->data;
	int ret;

	if (!gpio_is_ready_dt(&cfg->ec_pwrbtn)) {
		LOG_ERR_DEVICE_NOT_READY(cfg->ec_pwrbtn.port);
		return -ENODEV;
	}

	data->dev = dev;
	k_work_init_delayable(&data->release_work, ec_pwrbtn_release_handler);

	ret = gpio_pin_configure_dt(&cfg->ec_pwrbtn, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure EC power button GPIO: %d", ret);
		return ret;
	}

	return 0;
}

static const struct ec_pwrbtn_config ec_pwrbtn_cfg = {
	.ec_pwrbtn = GPIO_DT_SPEC_INST_GET(0, ec_pwrbtn_gpios),
};
static struct ec_pwrbtn_data ec_pwrbtn_data;
DEVICE_DT_INST_DEFINE(0, ec_pwrbtn_init, NULL, &ec_pwrbtn_data, &ec_pwrbtn_cfg,
		      POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY, NULL);
