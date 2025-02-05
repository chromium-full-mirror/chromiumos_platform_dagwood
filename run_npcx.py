#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Load an NPCX image into RAM and execute it"""

import argparse
import os
from pathlib import Path
import sys

# pylint: disable=import-error
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
import usb


args = None
target = None


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

UUT = "uartupdatetool"


def main(argv):
    global args
    global target

    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", type=str, default="ttyACM1")
    parser.add_argument("-b", "--board", type=str, required=True)
    args = parser.parse_args(argv)

    board_path = Path("build", "zephyr", args.board)
    elf_file = board_path / "build-ro" / "zephyr" / "zephyr.elf"
    bin_file = board_path / "build-ro" / "zephyr" / "zephyr.bin"
    monitor_file = board_path / "output" / "npcx_monitor.bin"

    print(f"Board ELF: {elf_file}")
    print(f"Board Bin: {bin_file}")

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

    rom_start = hex(rom_start)

    print(f"Entry point: {entry}")
    print(f"ROM start {rom_start}")

    dev = find_usb_device()

    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 1, USB_REQ_POWER, 0)

    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 0, USB_REQ_NPCX_BOOT, 0)

    # Load the monitor code
    os.system(
        f"{UUT} --port={args.port} --baudrate=115200 --opr=wr --addr=0x200C3020 --file {monitor_file}"
    )

    # Load the binary file directly to RAM
    os.system(
        f"{UUT} --port={args.port} --baudrate=115200 --opr=wr --addr={rom_start} --file {bin_file}"
    )

    # Jump to the entry point in RAM
    os.system(
        f"{UUT} --port={args.port} --baudrate=115200 --opr=go --addr={entry}"
    )


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
