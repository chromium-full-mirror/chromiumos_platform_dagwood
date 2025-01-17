/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define DT_DRV_COMPAT cros_npcx_boot

#include "ec.h"
#include "usb_request.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>

LOG_MODULE_REGISTER(npcx_boot, LOG_LEVEL_INF);

BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
	     "only one 'cros,npcx-boot' compatible node may be present");

#define PINCTRL_STATE_GPIO 2

PINCTRL_DT_INST_DEFINE(0);
const struct pinctrl_dev_config *npcx_boot_pcfg =
	PINCTRL_DT_INST_DEV_CONFIG_GET(0);
const struct gpio_dt_spec npcx_boot_gpio =
	GPIO_DT_SPEC_INST_GET(0, npcx_boot_gpios);

static int cmd_npcx_boot(const struct shell *sh, size_t argc, char **argv)
{
	int ret;

	ret = pinctrl_apply_state(npcx_boot_pcfg, PINCTRL_STATE_GPIO);
	if (ret < 0) {
		shell_error(sh, "1 pinctrl configuration failed: %d", ret);
		return ret;
	}

	ret = gpio_pin_configure_dt(&npcx_boot_gpio, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		shell_error(sh, "gpio configuration failed: %d", ret);
		return ret;
	}

	ec_reset();

	k_sleep(K_MSEC(100));

	ret = gpio_pin_configure_dt(&npcx_boot_gpio, GPIO_INPUT);
	if (ret < 0) {
		shell_error(sh, "gpio configuration failed: %d", ret);
		return ret;
	}

	ret = pinctrl_apply_state(npcx_boot_pcfg, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		shell_error(sh, "2 pinctrl configuration failed: %d", ret);
		return ret;
	}

	return 0;
}

SHELL_CMD_REGISTER(npcx_boot, NULL, "Enter NPCX bootloader", cmd_npcx_boot);

static void npcx_boot_cb(uint16_t index, uint16_t value)
{
	const struct shell *sh = shell_backend_uart_get_ptr();

	if (index != USB_REQ_NPCX_BOOT) {
		return;
	}

	LOG_INF("npcx boot");

	cmd_npcx_boot(sh, 0, NULL);
}

USB_REQUEST_CALLBACK_DEFINE(npcx_boot_cb);
