/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

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
