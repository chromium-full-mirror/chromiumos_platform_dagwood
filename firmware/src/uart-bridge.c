/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/usb/usb_device.h>

#define DT_DRV_COMPAT zephyr_uart_bridge

LOG_MODULE_REGISTER(uart_bridge, LOG_LEVEL_INF);

#define RING_BUF_SIZE 256
#define RING_BUF_FULL_THRESHOLD (RING_BUF_SIZE / 3)

struct uart_bridge_peer_config {
	const struct device *dev;
	size_t transfer_size;
};

struct uart_bridge_config {
	struct uart_bridge_peer_config peer[2];
};

struct uart_bridge_peer_data {
	uint8_t buf[RING_BUF_SIZE];
	struct ring_buf rb;
	bool paused;
};

struct uart_bridge_data {
	struct uart_bridge_peer_data peer[2];
};

static const struct device *
uart_bridge_get_peer(const struct device *dev, const struct device *bridge_dev)
{
	const struct uart_bridge_config *cfg = bridge_dev->config;

	if (dev == cfg->peer[0].dev) {
		return cfg->peer[1].dev;
	} else if (dev == cfg->peer[1].dev) {
		return cfg->peer[0].dev;
	} else {
		return NULL;
	}
}

void uart_bridge_settings_update(const struct device *dev,
				 const struct device *bridge_dev)
{
	struct uart_config cfg;
	const struct device *peer_dev = uart_bridge_get_peer(dev, bridge_dev);
	int ret;

	if (peer_dev == NULL) {
		LOG_DBG("%s: not a bridge dev", dev->name);
		return;
	}

	LOG_DBG("update settings: dev=%s bridge=%s peer=%s", dev->name,
		bridge_dev->name, peer_dev->name);

	ret = uart_config_get(dev, &cfg);
	if (ret) {
		LOG_WRN("%s: failed to get the uart config: %d", dev->name,
			ret);
		return;
	}

	ret = uart_configure(peer_dev, &cfg);
	if (ret) {
		LOG_WRN("%s: failed to set the uart config: %d", peer_dev->name,
			ret);
		return;
	}

	LOG_INF("uart settings: baudrate=%d parity=%d", cfg.baudrate,
		cfg.parity);
}

static uint8_t uart_bridge_get_idx(const struct device *dev,
				   const struct device *bridge_dev, bool own)
{
	const struct uart_bridge_config *cfg = bridge_dev->config;

	if (dev == cfg->peer[0].dev) {
		return own ? 0 : 1;
	} else {
		return own ? 1 : 0;
	}
}

static void uart_bridge_handle_rx(const struct device *dev,
				  const struct device *bridge_dev)
{
	const struct uart_bridge_config *cfg = bridge_dev->config;
	struct uart_bridge_data *data = bridge_dev->data;

	const struct uart_bridge_peer_config *own_cfg =
		&cfg->peer[uart_bridge_get_idx(dev, bridge_dev, true)];
	const struct uart_bridge_peer_config *peer_cfg =
		&cfg->peer[uart_bridge_get_idx(dev, bridge_dev, false)];
	struct uart_bridge_peer_data *own_data =
		&data->peer[uart_bridge_get_idx(dev, bridge_dev, true)];

	uint8_t buffer[own_cfg->transfer_size];
	int rb_len, recv_len;

	if (ring_buf_space_get(&own_data->rb) < RING_BUF_FULL_THRESHOLD) {
		LOG_DBG("%s: buffer full: pause", dev->name);
		uart_irq_rx_disable(dev);
		own_data->paused = true;
		return;
	}

	recv_len = uart_fifo_read(dev, buffer, sizeof(buffer));
	if (!recv_len)
		return;

	rb_len = ring_buf_put(&own_data->rb, buffer, recv_len);
	if (rb_len < recv_len) {
		LOG_WRN("%s: rx drop %u bytes", dev->name, recv_len - rb_len);
	}

	uart_irq_tx_enable(peer_cfg->dev);
}

static void uart_bridge_handle_tx(const struct device *dev,
				  const struct device *bridge_dev)
{
	const struct uart_bridge_config *cfg = bridge_dev->config;
	struct uart_bridge_data *data = bridge_dev->data;

	const struct uart_bridge_peer_config *own_cfg =
		&cfg->peer[uart_bridge_get_idx(dev, bridge_dev, true)];
	const struct uart_bridge_peer_config *peer_cfg =
		&cfg->peer[uart_bridge_get_idx(dev, bridge_dev, false)];
	struct uart_bridge_peer_data *peer_data =
		&data->peer[uart_bridge_get_idx(dev, bridge_dev, false)];

	uint8_t buffer[own_cfg->transfer_size];
	int rb_len, send_len;

	rb_len = ring_buf_get(&peer_data->rb, buffer, sizeof(buffer));
	if (!rb_len) {
		LOG_DBG("%s: buffer empty, disable tx irq", dev->name);
		uart_irq_tx_disable(dev);
		return;
	}

	send_len = uart_fifo_fill(dev, buffer, rb_len);
	if (send_len < rb_len) {
		LOG_WRN("%s: tx dropped %d bytes", dev->name,
			rb_len - send_len);
	}

	if (peer_data->paused &&
	    ring_buf_space_get(&peer_data->rb) > RING_BUF_FULL_THRESHOLD) {
		LOG_DBG("%s: buffer free: resume", dev->name);
		uart_irq_rx_enable(peer_cfg->dev);
		peer_data->paused = false;
		return;
	}
}

static void interrupt_handler(const struct device *dev, void *user_data)
{
	const struct device *bridge_dev = user_data;

	while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
		if (uart_irq_rx_ready(dev)) {
			uart_bridge_handle_rx(dev, bridge_dev);
		}
		if (uart_irq_tx_ready(dev)) {
			uart_bridge_handle_tx(dev, bridge_dev);
		}
	}
}

static int uart_bridge_init(const struct device *dev)
{
	const struct uart_bridge_config *cfg = dev->config;
	struct uart_bridge_data *data = dev->data;

	ring_buf_init(&data->peer[0].rb, RING_BUF_SIZE, data->peer[0].buf);
	ring_buf_init(&data->peer[1].rb, RING_BUF_SIZE, data->peer[1].buf);

	/* Wait 100ms for the host to do all settings */
	k_msleep(100);

	uart_irq_callback_user_data_set(cfg->peer[0].dev, interrupt_handler,
					(void *)dev);
	uart_irq_callback_user_data_set(cfg->peer[1].dev, interrupt_handler,
					(void *)dev);
	uart_irq_rx_enable(cfg->peer[0].dev);
	uart_irq_rx_enable(cfg->peer[1].dev);

	return 0;
}

#define UART_BRIDGE_PEER_DEFINE(node_id)                           \
	{                                                          \
		.dev = DEVICE_DT_GET(DT_PHANDLE(node_id, device)), \
		.transfer_size = DT_PROP(node_id, transfer_size),  \
	}

#define UART_BRIDGE_INIT(n)                                                \
	BUILD_ASSERT(DT_INST_CHILD_NUM(n) == 2,                            \
		     "uart-bridge must have exactly 2 child nodes");       \
                                                                           \
	static const struct uart_bridge_config uart_bridge_cfg_##n = {     \
		.peer = { DT_INST_FOREACH_CHILD_STATUS_OKAY_SEP(           \
			n, UART_BRIDGE_PEER_DEFINE, (, )) },               \
	};                                                                 \
                                                                           \
	static struct uart_bridge_data uart_bridge_data_##n;               \
                                                                           \
	DEVICE_DT_INST_DEFINE(n, uart_bridge_init, NULL(n),                \
			      &uart_bridge_data_##n, &uart_bridge_cfg_##n, \
			      POST_KERNEL, CONFIG_SERIAL_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(UART_BRIDGE_INIT)
