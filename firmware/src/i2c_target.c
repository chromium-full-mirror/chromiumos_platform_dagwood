/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <dagwood/i2c_target.h>

#define DT_DRV_COMPAT cros_i2c_target_memory

LOG_MODULE_REGISTER(dagwood_i2c_target, LOG_LEVEL_INF);

#define I2C_STRETCH_50MS_US (50 * USEC_PER_MSEC)
#define I2C_STRETCH_100MS_US (100 * USEC_PER_MSEC)
#define I2C_STRETCH_500MS_US (500 * USEC_PER_MSEC)

/* clang-format off */
const struct device *cros_i2c_target_memory_devs[] = {
	DT_FOREACH_STATUS_OKAY(cros_i2c_target_memory, DEVICE_DT_GET_COMMA)
};
/* clang-format on */

struct i2c_target_memory_config {
	struct i2c_dt_spec bus;
	uint32_t buffer_size;
};

struct i2c_target_memory_data {
	struct i2c_target_config config;
	const struct device *dev;
	uint32_t buffer_idx;
	uint32_t idx_write_cnt;
	uint8_t *buffer;
};

static int i2c_target_memory_write_requested(struct i2c_target_config *config)
{
	struct i2c_target_memory_data *data =
		CONTAINER_OF(config, struct i2c_target_memory_data, config);

	data->idx_write_cnt = 0;
	return 0;
}

static int i2c_target_memory_write_received(struct i2c_target_config *config,
					    uint8_t val)
{
	struct i2c_target_memory_data *data =
		CONTAINER_OF(config, struct i2c_target_memory_data, config);
	const struct i2c_target_memory_config *cfg = data->dev->config;

	if (data->idx_write_cnt == 0) {
		data->buffer_idx = val % cfg->buffer_size;
		data->idx_write_cnt++;
	} else {
		data->buffer[data->buffer_idx] = val;
		data->buffer_idx = (data->buffer_idx + 1) % cfg->buffer_size;
		data->idx_write_cnt++;
	}

	return 0;
}

static int i2c_target_memory_read_requested(struct i2c_target_config *config,
					    uint8_t *val)
{
	struct i2c_target_memory_data *data =
		CONTAINER_OF(config, struct i2c_target_memory_data, config);

	/* Note - the I2C target callbacks run in ISR context.
	 * Use k_busy_wait() to perform I2C clock stretching at
	 * specific register offsets.  The I2C bus used for target mode
	 * should lower it's interrupt priority below the EC UART channel
	 * so that EC console output is not dropped.
	 */
	if (data->buffer_idx == I2C_STRETCH_50MS_OFFSET) {
		k_busy_wait(I2C_STRETCH_50MS_US);
	} else if (data->buffer_idx == I2C_STRETCH_100MS_OFFSET) {
		k_busy_wait(I2C_STRETCH_100MS_US);
	} else if (data->buffer_idx == I2C_STRETCH_500MS_OFFSET) {
		k_busy_wait(I2C_STRETCH_500MS_US);
	}

	*val = data->buffer[data->buffer_idx];
	return 0;
}

static int i2c_target_memory_read_processed(struct i2c_target_config *config,
					    uint8_t *val)
{
	struct i2c_target_memory_data *data =
		CONTAINER_OF(config, struct i2c_target_memory_data, config);
	const struct i2c_target_memory_config *cfg = data->dev->config;

	data->buffer_idx = (data->buffer_idx + 1) % cfg->buffer_size;
	*val = data->buffer[data->buffer_idx];
	return 0;
}

static int i2c_target_memory_stop(struct i2c_target_config *config)
{
	struct i2c_target_memory_data *data =
		CONTAINER_OF(config, struct i2c_target_memory_data, config);

	data->idx_write_cnt = 0;
	return 0;
}

static const struct i2c_target_callbacks i2c_target_memory_callbacks = {
	.write_requested = i2c_target_memory_write_requested,
	.read_requested = i2c_target_memory_read_requested,
	.write_received = i2c_target_memory_write_received,
	.read_processed = i2c_target_memory_read_processed,
	.stop = i2c_target_memory_stop,
};

static int i2c_target_memory_register(const struct device *dev)
{
	const struct i2c_target_memory_config *cfg = dev->config;
	struct i2c_target_memory_data *data = dev->data;

	return i2c_target_register(cfg->bus.bus, &data->config);
}

static int i2c_target_memory_unregister(const struct device *dev)
{
	const struct i2c_target_memory_config *cfg = dev->config;
	struct i2c_target_memory_data *data = dev->data;

	return i2c_target_unregister(cfg->bus.bus, &data->config);
}

static DEVICE_API(i2c_target, i2c_target_memory_api) = {
	.driver_register = i2c_target_memory_register,
	.driver_unregister = i2c_target_memory_unregister,
};

static int i2c_target_memory_init(const struct device *dev)
{
	const struct i2c_target_memory_config *cfg = dev->config;
	struct i2c_target_memory_data *data = dev->data;
	int ret;

	if (!i2c_is_ready_dt(&cfg->bus)) {
		LOG_ERR("I2C bus %s not ready", cfg->bus.bus->name);
		return -ENODEV;
	}

	data->dev = dev;
	data->config.address = cfg->bus.addr;
	data->config.callbacks = &i2c_target_memory_callbacks;

	ret = i2c_target_register(cfg->bus.bus, &data->config);
	if (ret < 0) {
		LOG_ERR("Failed to register I2C target: %d", ret);
		return ret;
	}

	LOG_INF("I2C target memory device registered on %s:0x%02x (size %u)",
		cfg->bus.bus->name, cfg->bus.addr, cfg->buffer_size);
	return 0;
}

static int reset_target_device(const struct device *dev)
{
	const struct i2c_target_memory_config *cfg = dev->config;
	struct i2c_target_memory_data *data = dev->data;
	int ret;

	ret = i2c_target_unregister(cfg->bus.bus, &data->config);
	if (ret < 0) {
		LOG_WRN("i2c_target_unregister failed: %d", ret);
	}

	data->buffer_idx = 0;
	data->idx_write_cnt = 0;
	memset(data->buffer, 0, cfg->buffer_size);

	ret = i2c_target_register(cfg->bus.bus, &data->config);
	if (ret < 0) {
		LOG_ERR("i2c_target_register failed: %d", ret);
		return ret;
	}

	return 0;
}

static int cmd_i2c_target_reset(const struct shell *sh, size_t argc,
				char **argv)
{
	if (argc > 1) {
		const struct device *dev = device_get_binding(argv[1]);

		if (dev == NULL) {
			shell_error(sh, "Device %s not found", argv[1]);
			return -ENODEV;
		}

		if (reset_target_device(dev) < 0) {
			shell_error(sh, "Failed to reset %s", argv[1]);
			return -EIO;
		}

		shell_print(sh, "Reset %s successfully", dev->name);
		return 0;
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(cros_i2c_target_memory_devs); i++) {
		const struct device *dev = cros_i2c_target_memory_devs[i];
		if (device_is_ready(dev)) {
			if (reset_target_device(dev) == 0) {
				shell_print(sh, "Reset %s successfully",
					    dev->name);
			} else {
				shell_error(sh, "Failed to reset %s",
					    dev->name);
			}
		}
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	i2c_target_cmds,
	SHELL_CMD_ARG(reset, NULL, "reset [device] - Reset I2C target state",
		      cmd_i2c_target_reset, 1, 1),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(i2c_target, &i2c_target_cmds, "I2C target commands", NULL);

#define I2C_TARGET_MEMORY_INIT(inst)                                           \
	static uint8_t i2c_target_memory_buffer_##inst[DT_INST_PROP_OR(        \
		inst, size, 256)];                                             \
	static struct i2c_target_memory_data i2c_target_memory_data_##inst = { \
		.buffer = i2c_target_memory_buffer_##inst,                     \
	};                                                                     \
	static const struct i2c_target_memory_config                           \
		i2c_target_memory_config_##inst = {                            \
			.bus = I2C_DT_SPEC_INST_GET(inst),                     \
			.buffer_size = DT_INST_PROP_OR(inst, size, 256),       \
		};                                                             \
	DEVICE_DT_INST_DEFINE(inst, &i2c_target_memory_init, NULL,             \
			      &i2c_target_memory_data_##inst,                  \
			      &i2c_target_memory_config_##inst, POST_KERNEL,   \
			      CONFIG_I2C_TARGET_INIT_PRIORITY,                 \
			      &i2c_target_memory_api);

DT_INST_FOREACH_STATUS_OKAY(I2C_TARGET_MEMORY_INIT)
