#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import serial.tools.list_ports
import usb


DAGWOOD_PORT_INFO = "Dagwood - Dagwood shell"
DAGWOOD_EC_PORT_INFO = "Dagwood - EC shell"


def find_default_ec_port():
    for port in serial.tools.list_ports.comports():
        if port.usb_description() == DAGWOOD_EC_PORT_INFO:
            return port.device

    # Fallback to ttyACM1, should be correct if there's no other ACM devices.
    return "/dev/ttyACM1"


def find_usb_device():
    dev = usb.core.find(manufacturer="Google", product="Dagwood")
    if dev is None:
        raise ValueError("Device not found")

    return dev


# bRequest values:
# bit 7 = 0: host to device
# bit 6..5 = vendor
# bit 4..0 = device
TO_DEV_REQ_TYPE = 0x40
# bit 7 = 0: device to host
# bit 6..5 = vendor
# bit 4..0 = device
TO_HOST_REQ_TYPE = 0xC0

# Must match the definitions in firmware/src/usb_request.h
USB_REQ_EC_RESET = 0
USB_REQ_POWER = 1
USB_REQ_NPCX_BOOT = 2

# NPXC uartupdatetool name
UUT = "uartupdatetool"

# ITE UART update tool name
ITEFLASH = "itecomdbgr"


def dw_req_power(dev, enable):
    """Control the EC board power"""
    if enable:
        value = 1
    else:
        value = 0

    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, value, USB_REQ_POWER, 0)


def dw_ec_reset(dev):
    """Resets the EC board"""
    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 0, USB_REQ_EC_RESET, 0)


def dw_req_npcx_boot(dev):
    """Put an NPCX EC board in boot mode"""
    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 0, USB_REQ_NPCX_BOOT, 0)
