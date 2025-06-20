/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "usb_request.h"

#include <zephyr/drivers/uart/uart_bridge.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device_runtime.h>

LOG_MODULE_REGISTER(uart_switch, LOG_LEVEL_INF);

static void ec_uart_select_alt(uint16_t index, uint16_t value)
{
	static bool ec_uart_alt;

	if (index != USB_REQ_EC_UART_SELECT_ALT) {
		return;
	}

	value = !!value;

	if (value == ec_uart_alt) {
		return;
	}

	LOG_INF("ec uart select: alt=%d", value);

	if (value) {
		pm_device_runtime_put(DEVICE_DT_GET(DT_NODELABEL(uart_bridge)));
		pm_device_runtime_get(
			DEVICE_DT_GET(DT_NODELABEL(uart_bridge_alt)));

		uart_bridge_settings_update(
			DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart_ec_alt)),
			DEVICE_DT_GET(DT_NODELABEL(uart_bridge_alt)));

	} else {
		pm_device_runtime_put(
			DEVICE_DT_GET(DT_NODELABEL(uart_bridge_alt)));
		pm_device_runtime_get(DEVICE_DT_GET(DT_NODELABEL(uart_bridge)));

		uart_bridge_settings_update(
			DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart_ec)),
			DEVICE_DT_GET(DT_NODELABEL(uart_bridge)));
	}

	ec_uart_alt = value;
}

USB_REQUEST_CALLBACK_DEFINE(ec_uart_select_alt);

static int ec_uart_select_init(void)
{
	int ret;

	ret = pm_device_runtime_get(DEVICE_DT_GET(DT_NODELABEL(uart_bridge)));
	if (ret < 0) {
		LOG_ERR("pm_device_runtime_get error: %d", ret);
		return ret;
	}

	return 0;
}

SYS_INIT(ec_uart_select_init, APPLICATION, 0);
