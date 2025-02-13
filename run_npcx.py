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

import utils


args = None
target = None


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

    dev = utils.find_usb_device()

    utils.dw_req_power(dev, True)
    utils.dw_req_npcx_boot(dev)

    # Load the monitor code
    os.system(
        f"{utils.UUT} --port={args.port} --baudrate=115200 --opr=wr --addr=0x200C3020 --file {monitor_file}"
    )

    # Load the binary file directly to RAM
    os.system(
        f"{utils.UUT} --port={args.port} --baudrate=115200 --opr=wr --addr={rom_start} --file {bin_file}"
    )

    # Jump to the entry point in RAM
    os.system(
        f"{utils.UUT} --port={args.port} --baudrate=115200 --opr=go --addr={entry}"
    )


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
