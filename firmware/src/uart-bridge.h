/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <zephyr/device.h>

/**
 * @brief Update the hardware port settings on an uart bridge
 *
 * If dev is part bridge_dev, then the dev uart configuration are applied to
 * the other device in the uart bridge. This allows propagating the settings
 * from a USB CDC-ACM port to a hardware UART.
 *
 * If dev is not part of bridge_dev then the function is a no-op.
 */
void uart_bridge_settings_update(const struct device *dev,
				 const struct device *bridge_dev);
