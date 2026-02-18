#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
from pathlib import Path
import shutil

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
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


def verify_executable(*filepaths):
    """Check if any of the specified commands exists, returns the first match."""
    for filepath in filepaths:
        if shutil.which(filepath):
            return filepath

        if os.path.isfile(filepath) and os.access(filepath, os.X_OK):
            return Path(filepath).resolve()

    raise ValueError(f"No such executable {filepaths}")


def elf_offsets(elf_file):
    """Find the ROM start and entry point for the specified elf_file"""
    with open(elf_file, "rb") as file:
        elf = ELFFile(file)
        entry = hex(elf.header["e_entry"])
        for section in elf.iter_sections():
            if isinstance(section, SymbolTableSection):
                symbols = {
                    s.name: s.entry.st_value for s in section.iter_symbols()
                }
                break

    rom_start = symbols.get("__rom_start_address")
    if rom_start is None:
        raise LookupError("__rom_start_address symbol not found")

    return entry, hex(rom_start)


def parse_arguments(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-b",
        "--board",
        type=str,
        required=False,
        help="zmake board name, this is typically a zmake project, the script "
        "is going to look for a build/zephyr/<board-dir> path in the current "
        "directory.",
    )
    parser.add_argument(
        "-d",
        "--build-dir",
        type=Path,
        required=False,
        help="Build directory path, this points to the output directory of a "
        "Zephyr build, can be used to flash an application that has not been "
        "build with zmake, or to only flash a ro image by pointing at the "
        "build-ro directory from a zmake build.",
    )
    parser.add_argument(
        "--ro",
        action="store_true",
        help="Flash a zephyr_ro.bin image if available. This is much faster "
        "than flashing the whole ro/rw ec.bin file, can be useful for "
        "debugging.",
    )
    parser.add_argument(
        "--board-id",
        type=str,
        required=False,
        help="Dagwood board serial number",
    )
    parser.add_argument(
        "-r",
        "--run",
        action="store_true",
        help="Load and run the image from SRAM instead of flash, ignored if not supported",
    )
    parser.add_argument(
        "--reboot",
        action="store_true",
        help="Reboot the dagwood board",
    )
    parser.add_argument(
        "--no-preserve",
        action="store_true",
        help="Avoid preserving the sections of firmware image with preserve property",
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
    try:
        dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 0, USB_REQ_REBOOT, 0)
    except usb.USBError as e:
        # This is expected, ignore
        pass
