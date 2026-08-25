/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "usb_request.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart/uart_bridge.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/net_buf.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/minmax.h>
#include <zephyr/usb/usbd.h>

LOG_MODULE_REGISTER(usbd_app, LOG_LEVEL_INF);

#define USB_VID 0x18d1
#define USB_PID 0x5214
#define USB_MANUFACTURER_NAME "Google"
#define USB_DEVICE_NAME "Dagwood"
#define USB_MAX_POWER 250

USBD_DEVICE_DEFINE(app_usbd, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)), USB_VID,
		   USB_PID);

USBD_DESC_LANG_DEFINE(app_usb_lang);
USBD_DESC_MANUFACTURER_DEFINE(app_usb_mfr, USB_MANUFACTURER_NAME);
USBD_DESC_PRODUCT_DEFINE(app_usb_product, USB_DEVICE_NAME);
USBD_DESC_SERIAL_NUMBER_DEFINE(app_usb_sn);

static const uint8_t attributes = 0;

USBD_DESC_CONFIG_DEFINE(fs_cfg_desc, "FS Configuration");
USBD_DESC_CONFIG_DEFINE(hs_cfg_desc, "HS Configuration");
USBD_CONFIGURATION_DEFINE(app_usb_fs_config, attributes, USB_MAX_POWER,
			  &fs_cfg_desc);
USBD_CONFIGURATION_DEFINE(app_usb_hs_config, attributes, USB_MAX_POWER,
			  &hs_cfg_desc);

static struct net_buf *to_host_cb(const struct usbd_context *const ctx,
				  const struct usb_setup_packet *const setup)
{
	static uint8_t host_buf[USB_REQUEST_MAX_BUF_SIZE];
	struct net_buf *buf;
	size_t len;
	int ret;

	STRUCT_SECTION_FOREACH(usb_request_callback, callback)
	{
		if (callback->to_host == NULL) {
			continue;
		}

		ret = callback->to_host(setup->wIndex, setup->wValue, host_buf,
					sizeof(host_buf));
		if (ret <= 0) {
			/* Skip if the callback returned no data or an error */
			continue;
		}

		len = min((size_t)ret, sizeof(host_buf));
		len = min(len, (size_t)setup->wLength);

		buf = usbd_ep_ctrl_data_in_alloc(ctx, len);
		if (buf == NULL) {
			LOG_ERR("Failed to allocate data IN buffer");
			return NULL;
		}

		net_buf_add_mem(buf, host_buf, len);

		return buf;
	}

	return NULL;
}

static int to_dev_cb(const struct usbd_context *const ctx,
		     const struct usb_setup_packet *const setup,
		     const struct net_buf *const buf)
{
	static uint8_t dev_buf[USB_REQUEST_MAX_BUF_SIZE];
	size_t len = 0;

	if (setup->wLength > sizeof(dev_buf)) {
		return -EINVAL;
	}

	if (setup->wLength > 0 && buf == NULL) {
		return 0;
	}

	if (buf != NULL) {
		len = net_buf_linearize(dev_buf, sizeof(dev_buf), buf, 0,
					buf->len);
	}

	STRUCT_SECTION_FOREACH(usb_request_callback, callback)
	{
		if (callback->to_dev != NULL) {
			callback->to_dev(setup->wIndex, setup->wValue,
					 len > 0 ? dev_buf : NULL, len);
		}
	}

	return 0;
}

USBD_VREQUEST_DEFINE(vnd_vreq, 0, to_host_cb, to_dev_cb);

static void update_bridge_settings_if_active(const struct device *dev,
					     const struct device *bridge_dev)
{
	if (pm_device_runtime_usage(bridge_dev) <= 0) {
		return;
	}

	uart_bridge_settings_update(dev, bridge_dev);
}

static void usbd_msg_cb(struct usbd_context *const usbd_ctx,
			const struct usbd_msg *const msg)
{
	int ret;

	LOG_DBG("USBD message: %s", usbd_msg_type_string(msg->type));

	if (msg->type == USBD_MSG_CDC_ACM_LINE_CODING ||
	    msg->type == USBD_MSG_CDC_ACM_CONTROL_LINE_STATE) {
		update_bridge_settings_if_active(
			msg->dev, DEVICE_DT_GET(DT_NODELABEL(uart_bridge)));
		update_bridge_settings_if_active(
			msg->dev, DEVICE_DT_GET(DT_NODELABEL(uart_bridge_alt)));
	}

	if (!usbd_can_detect_vbus(usbd_ctx)) {
		return;
	}

	switch (msg->type) {
	case USBD_MSG_VBUS_READY:
		ret = usbd_enable(usbd_ctx);
		if (ret) {
			LOG_ERR("Failed to enable device support");
		}

		break;
	case USBD_MSG_VBUS_REMOVED:
		ret = usbd_disable(usbd_ctx);
		if (ret) {
			LOG_ERR("Failed to disable device support");
		}

		break;
	case USBD_MSG_RESUME:
		break;
	case USBD_MSG_SUSPEND:
		break;
	default:
		break;
	}
}

static struct usbd_context *usbd_init_device(void)
{
	int err;

	err = usbd_add_descriptor(&app_usbd, &app_usb_lang);
	if (err) {
		LOG_ERR("Failed to initialize language descriptor (%d)", err);
		return NULL;
	}

	err = usbd_add_descriptor(&app_usbd, &app_usb_mfr);
	if (err) {
		LOG_ERR("Failed to initialize manufacturer descriptor (%d)",
			err);
		return NULL;
	}

	err = usbd_add_descriptor(&app_usbd, &app_usb_product);
	if (err) {
		LOG_ERR("Failed to initialize product descriptor (%d)", err);
		return NULL;
	}

	err = usbd_add_descriptor(&app_usbd, &app_usb_sn);
	if (err) {
		LOG_ERR("Failed to initialize SN descriptor (%d)", err);
		return NULL;
	}

	if (usbd_caps_speed(&app_usbd) == USBD_SPEED_HS) {
		err = usbd_add_configuration(&app_usbd, USBD_SPEED_HS,
					     &app_usb_hs_config);
		if (err) {
			LOG_ERR("Failed to add High-Speed configuration");
			return NULL;
		}

		err = usbd_register_all_classes(&app_usbd, USBD_SPEED_HS, 1,
						NULL);
		if (err) {
			LOG_ERR("Failed to add register classes");
			return NULL;
		}

		usbd_device_set_code_triple(&app_usbd, USBD_SPEED_HS, 0, 0, 0);
	}

	err = usbd_add_configuration(&app_usbd, USBD_SPEED_FS,
				     &app_usb_fs_config);
	if (err) {
		LOG_ERR("Failed to add Full-Speed configuration");
		return NULL;
	}

	err = usbd_register_all_classes(&app_usbd, USBD_SPEED_FS, 1, NULL);
	if (err) {
		LOG_ERR("Failed to add register classes");
		return NULL;
	}

	usbd_device_set_code_triple(&app_usbd, USBD_SPEED_FS, 0, 0, 0);

	err = usbd_msg_register_cb(&app_usbd, usbd_msg_cb);
	if (err) {
		LOG_ERR("Failed to register message callback");
		return NULL;
	}

	err = usbd_device_register_vreq(&app_usbd, &vnd_vreq);
	if (err) {
		LOG_ERR("Failed to register vreq");
		return NULL;
	}

	err = usbd_init(&app_usbd);
	if (err) {
		LOG_ERR("Failed to initialize device support");
		return NULL;
	}

	return &app_usbd;
}

static int app_usbd_enable(void)
{
	struct usbd_context *usbd;
	int ret;

	usbd = usbd_init_device();
	if (usbd == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -ENODEV;
	}

	if (usbd_can_detect_vbus(usbd)) {
		return 0;
	}

	ret = usbd_enable(usbd);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return 0;
	}

	return 0;
}
SYS_INIT(app_usbd_enable, APPLICATION, 0);
