/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define DT_DRV_COMPAT cros_ec_acok

#include "../usb_request.h"
#include "ec_acok.h"

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(ec_acok, LOG_LEVEL_INF);

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
	     "only one 'cros,ec-acok' compatible node may be present");

struct ec_acok_config {
	struct gpio_dt_spec acok;
};

static const struct device *const acok_dev = DEVICE_DT_INST_GET(0);

int ec_acok_set_state(bool enable)
{
	const struct ec_acok_config *cfg = acok_dev->config;

	if (!device_is_ready(acok_dev)) {
		return -ENODEV;
	}

	return gpio_pin_set_dt(&cfg->acok, enable ? 1 : 0);
}

int ec_acok_get_state(void)
{
	const struct ec_acok_config *cfg = acok_dev->config;

	if (!device_is_ready(acok_dev)) {
		return 0;
	}

	return gpio_pin_get_dt(&cfg->acok);
}

static int cmd_acok(const struct shell *sh, size_t argc, char **argv)
{
	if (argc >= 2) {
		bool enable;
		int rv = 0;

		enable = shell_strtobool(argv[1], 0, &rv);
		if (rv < 0) {
			shell_error(sh, "Failed to parse ACOK state: %d", rv);
			return rv;
		}
		ec_acok_set_state(enable);
	}

	shell_print(sh, "ACOK: %s", ec_acok_get_state() ? "ON" : "OFF");
	return 0;
}

SHELL_CMD_ARG_REGISTER(acok, NULL, "Set/get ACOK: acok [on|off|1|0]", cmd_acok,
		       1, 1);

static void ec_acok_to_dev(uint16_t index, uint16_t value, const uint8_t *buf,
			   size_t size)
{
	ARG_UNUSED(buf);
	ARG_UNUSED(size);

	if (index != USB_REQ_AP_SET_ACOK) {
		return;
	}

	ec_acok_set_state(value != 0);
}

USB_REQUEST_CALLBACK_DEFINE(ec_acok_to_dev, NULL);

static int ec_acok_init(const struct device *dev)
{
	const struct ec_acok_config *cfg = dev->config;
	int ret;

	if (!gpio_is_ready_dt(&cfg->acok)) {
		LOG_ERR_DEVICE_NOT_READY(cfg->acok.port);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&cfg->acok, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure ACOK GPIO: %d", ret);
		return ret;
	}

	return 0;
}

static const struct ec_acok_config ec_acok_cfg = {
	.acok = GPIO_DT_SPEC_INST_GET(0, acok_gpios),
};

DEVICE_DT_INST_DEFINE(0, ec_acok_init, NULL, NULL, &ec_acok_cfg, POST_KERNEL,
		      CONFIG_APPLICATION_INIT_PRIORITY, NULL);
