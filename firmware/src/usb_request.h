/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdint.h>

#define USB_REQ_EC_RESET 0
#define USB_REQ_POWER 1
#define USB_REQ_NPCX_BOOT 2
#define USB_REQ_EC_UART_SELECT_ALT 3
#define USB_REQ_REBOOT 4

struct usb_request_callback {
	void (*callback)(uint16_t index, uint16_t value);
};

#define USB_REQUEST_CALLBACK_DEFINE(_callback)                                \
	static const STRUCT_SECTION_ITERABLE(                                 \
		usb_request_callback, _usb_request_callback__##_callback) = { \
		.callback = _callback,                                        \
	}
