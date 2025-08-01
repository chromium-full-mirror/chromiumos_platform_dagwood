#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Load an NPCX image into RAM and execute it"""

import os
from pathlib import Path
import subprocess
import sys

# pylint: disable=import-error
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

import utils


args = None


def main(argv):
    global args
    args = utils.parse_arguments(argv)

    board_path = Path("build", "zephyr", args.board)
    elf_file = board_path / "build-ro" / "zephyr" / "zephyr.elf"
    bin_file = board_path / "build-ro" / "zephyr" / "zephyr.bin"

    utils.verify_file(bin_file)

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

    dev = utils.find_usb_device(args.board_id)
    port_base = os.path.basename(utils.find_programming_port(dev))

    utils.dw_req_power(dev, True)
    utils.dw_ec_uart_select_alt(dev, True)
    utils.dw_req_npcx_boot(dev)

    # Load the binary file directly to RAM
    subprocess.run(
        [
            utils.UUT,
            f"--port={port_base}",
            "--baudrate=115200",
            "--opr=wr",
            f"--addr={rom_start}",
            f"--file={bin_file}",
        ],
        check=True,
        env=os.environ,
    )

    # Jump to the entry point in RAM
    subprocess.run(
        [
            utils.UUT,
            f"--port={port_base}",
            "--baudrate=115200",
            "--opr=go",
            f"--addr={entry}",
        ],
        check=True,
        env=os.environ,
    )

    utils.dw_ec_uart_select_alt(dev, False)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
