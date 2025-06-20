/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#define DT_DRV_COMPAT cros_i2c_mux

LOG_MODULE_REGISTER(i2c_mux, LOG_LEVEL_INF);

struct i2c_mux_config {
	const struct gpio_dt_spec en_gpio;
	const struct gpio_dt_spec *a_gpio;
	uint8_t a_gpio_count;
	uint8_t port_count;
};

static int i2c_mux_select(const struct device *dev, uint8_t port)
{
	const struct i2c_mux_config *cfg = dev->config;
	int ret;

	if (port >= cfg->port_count) {
		return -EINVAL;
	}

	for (uint8_t i = 0; i < cfg->a_gpio_count; i++) {
		const struct gpio_dt_spec *gpio = &cfg->a_gpio[i];
		uint8_t val = IS_BIT_SET(port, i) != 0 ? 1 : 0;

		ret = gpio_pin_set_dt(gpio, val);
		if (ret < 0) {
			LOG_ERR("gpio set failed: %d", ret);
			return ret;
		}
	}

	return 0;
}

static const struct i2c_mux_api {
	/* empty, just for use in shell_device_filter */
} i2c_mux_api;

static bool i2c_mux_device_filter(const struct device *dev)
{
	return dev->api == &i2c_mux_api;
}

static void device_name_get(size_t idx, struct shell_static_entry *entry)
{
	const struct device *dev =
		shell_device_filter(idx, i2c_mux_device_filter);

	entry->syntax = (dev != NULL) ? dev->name : NULL;
	entry->handler = NULL;
	entry->help = NULL;
	entry->subcmd = NULL;
}

SHELL_DYNAMIC_CMD_CREATE(dsub_device_name, device_name_get);

static int cmd_i2c_mux(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev;
	int err = 0;
	int port;

	dev = device_get_binding(argv[1]);
	if (dev == NULL) {
		shell_error(sh, "Device %s not available", argv[1]);
		return -ENODEV;
	}

	port = shell_strtol(argv[2], 0, &err);
	if (err) {
		shell_error(sh, "Invalid argument: %s", argv[2]);
		return err;
	}

	err = i2c_mux_select(dev, port);
	if (err < 0) {
		shell_error(sh, "i2c_mux_select error: %d", err);
		return err;
	}

	return 0;
}

SHELL_CMD_ARG_REGISTER(i2c_mux, &dsub_device_name, "I2C multiplexer control",
		       cmd_i2c_mux, 3, 0);

static int i2c_mux_init(const struct device *dev)
{
	const struct i2c_mux_config *cfg = dev->config;
	int ret;

	ret = gpio_pin_configure_dt(&cfg->en_gpio, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("gpio configuration failed: %d", ret);
		return ret;
	}

	for (uint8_t i = 0; i < cfg->a_gpio_count; i++) {
		const struct gpio_dt_spec *gpio = &cfg->a_gpio[i];

		ret = gpio_pin_configure_dt(gpio, GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			LOG_ERR("gpio configuration failed: %d", ret);
			return ret;
		}
	}

	return 0;
}

#define I2C_MUX_INIT(n)                                                      \
	const struct gpio_dt_spec i2c_mux_a_gpio_##n[] = {                   \
		DT_INST_FOREACH_PROP_ELEM_SEP(n, a_gpios,                    \
					      GPIO_DT_SPEC_GET_BY_IDX, (, )) \
	};                                                                   \
                                                                             \
	static const struct i2c_mux_config i2c_mux_cfg_##n = {               \
		.en_gpio = GPIO_DT_SPEC_INST_GET(n, en_gpios),               \
		.a_gpio = i2c_mux_a_gpio_##n,                                \
		.a_gpio_count = ARRAY_SIZE(i2c_mux_a_gpio_##n),              \
		.port_count = DT_INST_PROP(n, port_count),                   \
	};                                                                   \
                                                                             \
	DEVICE_DT_INST_DEFINE(n, i2c_mux_init, NULL, NULL, &i2c_mux_cfg_##n, \
			      POST_KERNEL, CONFIG_I2C_INIT_PRIORITY,         \
			      &i2c_mux_api);

DT_INST_FOREACH_STATUS_OKAY(I2C_MUX_INIT)
