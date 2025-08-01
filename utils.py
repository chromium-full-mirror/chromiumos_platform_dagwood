#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os

import serial.tools.list_ports
import usb


DAGWOOD_PORT_INFO = "Dagwood - Dagwood shell"
DAGWOOD_EC_PORT_INFO = "Dagwood - EC shell"
DAGWOOD_EC_ALT_PORT_INFO = "Dagwood - EC UART programming"


def find_usb_device(board_id):
    device_spec = {
        "manufacturer": "Google",
        "product": "Dagwood",
    }

    if board_id:
        device_spec["serial_number"] = board_id

    dev = usb.core.find(**device_spec)
    if dev is None:
        raise ValueError("Device not found")

    print(f"Board id: {dev.serial_number}")

    return dev


def find_programming_port(dev):
    for port in serial.tools.list_ports.comports():
        if (
            port.serial_number == dev.serial_number
            and port.usb_description() == DAGWOOD_EC_ALT_PORT_INFO
        ):
            return port.device

    raise ValueError(f"No port found for device serial={dev.serial_number}")


def verify_file(*filepaths):
    """Check if any of the specified files exists, returns the first match."""
    for filepath in filepaths:
        if os.path.isfile(filepath):
            return filepath
    raise ValueError(f"No such file {filepaths}")


def parse_arguments(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-b", "--board", type=str, required=True, help="zmake board name"
    )
    parser.add_argument(
        "--board-id",
        type=str,
        required=False,
        help="Dagwood board serial number",
    )

    args = parser.parse_args(argv)

    return args


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
USB_REQ_EC_UART_SELECT = 3
USB_REQ_REBOOT = 4

# NPXC uartupdatetool name
UUT = "uartupdatetool"

# ITE UART update tool name
ITEFLASH = "itecomdbgr"

# Realtek UART update tool name
RTKFLASH = "rtkupdate"


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


def dw_ec_uart_select_alt(dev, enable):
    if enable:
        value = 1
    else:
        value = 0

    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, value, USB_REQ_EC_UART_SELECT, 0)


def dw_reboot(dev):
    """Reboot the Dagwood board"""
    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 0, USB_REQ_REBOOT, 0)
