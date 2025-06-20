/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/drivers/led.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

const struct led_dt_spec status_led = LED_DT_SPEC_GET(DT_NODELABEL(status_led));

int main(void)
{
	int ret;

	ret = pm_device_runtime_get(DEVICE_DT_GET(DT_NODELABEL(gpio_keys)));
	if (ret < 0) {
		LOG_ERR("pm_device_runtime_get error: %d", ret);
		return ret;
	}

	LOG_INF("started");

	while (true) {
		led_on_dt(&status_led);
		k_sleep(K_MSEC(1000));
		led_off_dt(&status_led);
		k_sleep(K_MSEC(1000));
	}
}

/* JLink seems to have issues flashing a board when it boots cleanly from
 * power-on and has never been flashed, holding the reset button by hand while
 * running "west flash" seems to temporarily fix the problem, the JLink does
 * reset the MCU while connecting so this seems some race condition in the CPU
 * boot process.
 *
 * Adding a short delay after sys_clock_driver_init(), which runs in
 * PRE_KERNEL_2 seems to work around the problem.
 */
static int jlink_helper_hack(void)
{
	k_busy_wait(100 * USEC_PER_MSEC);

	return 0;
}

SYS_INIT(jlink_helper_hack, PRE_KERNEL_2, 99);
