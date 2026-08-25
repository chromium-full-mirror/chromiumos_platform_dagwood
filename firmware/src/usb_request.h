/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stddef.h>
#include <stdint.h>

#define USB_REQ_EC_RESET 0
#define USB_REQ_POWER 1
#define USB_REQ_NPCX_BOOT 2
#define USB_REQ_EC_UART_SELECT_ALT 3
#define USB_REQ_REBOOT 4

#define USB_REQUEST_MAX_BUF_SIZE 128

struct usb_request_callback {
	void (*to_dev)(uint16_t index, uint16_t value, const uint8_t *buf,
		       size_t size);
	int (*to_host)(uint16_t index, uint16_t value, uint8_t *buf,
		       size_t size);
};

#define USB_REQUEST_CALLBACK_DEFINE(_to_dev, _to_host)             \
	static const STRUCT_SECTION_ITERABLE(                      \
		usb_request_callback,                              \
		_usb_request_callback__##_to_dev##_##_to_host) = { \
		.to_dev = _to_dev,                                 \
		.to_host = _to_host,                               \
	}
