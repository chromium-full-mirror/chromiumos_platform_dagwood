/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "usb_request.h"

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/minmax.h>
#include <zephyr/sys/util.h>

#include <dagwood/version.h>

static int cmd_version(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "%s", DW_VERSION);
	return 0;
}

SHELL_CMD_REGISTER(version, NULL, "Print firmware version", cmd_version);

static int version_to_host(uint16_t index, uint16_t value, uint8_t *buf,
			   size_t size)
{
	size_t len;

	if (index != USB_REQ_VERSION) {
		return 0;
	}

	len = min(strlen(DW_VERSION), size);
	memcpy(buf, DW_VERSION, len);

	return len;
}

USB_REQUEST_CALLBACK_DEFINE(NULL, version_to_host);
