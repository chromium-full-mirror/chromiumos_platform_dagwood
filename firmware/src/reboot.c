/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "usb_request.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

static void dw_reboot(uint16_t index, uint16_t value)
{
	if (index != USB_REQ_REBOOT) {
		return;
	}

	sys_reboot(SYS_REBOOT_COLD);
}

USB_REQUEST_CALLBACK_DEFINE(dw_reboot);
