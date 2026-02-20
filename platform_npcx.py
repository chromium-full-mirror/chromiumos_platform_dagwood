# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
from pathlib import Path
import subprocess

from platform_base import Platform

import utils


class PlatformNPCX(Platform):
    """NPCX platform class"""

    NAME = "NPCX"
    SDK_MONITOR_PATH = Path("/usr/share/ec-devutils/npcx_monitor.bin")
    NPCX_BASE_ADDR = 0x64000000
    UUT = "uartupdatetool"
    ZEPHYR_BIN_FILE = "zephyr.npcx.bin"

    def __init__(self, build_path, usb_dev, args):
        super().__init__(build_path, usb_dev, args)

        self.monitor_file = utils.verify_file(
            build_path / "build-singleimage" / "npcx_monitor.bin",
            self.SDK_MONITOR_PATH,
        )

        self.uut = utils.verify_executable(
            Path("build") / "host" / "util" / self.UUT,
            self.UUT,
        )

        self.port_base = os.path.basename(self.port)

        if self.args.run:
            elf_file = utils.verify_file(
                build_path / "build-ro" / "zephyr" / "zephyr.elf",
                build_path / "build-rw" / "zephyr" / "zephyr.elf",
                build_path / "build-singleimage" / "zephyr" / "zephyr.elf",
                build_path / "zephyr" / "zephyr.elf",
            )
            self.bin_file = elf_file.with_suffix(".bin")
            self.entry, self.rom_start = utils.elf_offsets(elf_file)
            print(
                f"File: {elf_file} entry point: {self.entry} ROM start: {self.rom_start}"
            )

    @staticmethod
    def match(board_name):
        return board_name in ["npcx9", "npcx7"]

    def enter_bootloader(self):
        utils.dw_req_npcx_boot(self.usb_dev)

        # If we're flashing, we want to load the monitor code before
        # all other steps.
        if not self.args.run:
            flash_monitor = [
                str(self.uut),
                f"--port={self.port_base}",
                "--baudrate=115200",
                "--opr=wr",
                "--addr=0x200C3020",
                f"--file={self.monitor_file}",
            ]
            print(f"Flashing monitor: {' '.join(flash_monitor)}")
            subprocess.run(flash_monitor, check=True)

    def read_partially(self, offset, size, file_path):
        command = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=rd",
            f"--addr={offset + self.NPCX_BASE_ADDR}",
            f"--size={size}",
            f"--file={file_path}",
        ]
        print(f"Reading: {' '.join(command)}")
        subprocess.run(command, check=True)

    def read_chip(self, file_path):
        command = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--read-flash",
            f"--file={file_path}",
        ]
        subprocess.run(command, check=True)

    def npcx_flash_commands(self):
        flash_image = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            "--auto",
            "--offset=0",
            f"--file={self.bin_file}",
        ]
        return [flash_image]

    def npcx_run_commands(self):
        load_image = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            f"--addr={self.rom_start}",
            f"--file={self.bin_file}",
        ]
        run_image = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=go",
            f"--addr={self.entry}",
        ]
        return [load_image, run_image]

    def flash_commands(self):

        if self.args.run:
            return self.npcx_run_commands()
        else:
            return self.npcx_flash_commands()

    def exit_bootloader(self):
        if not self.args.run:
            utils.dw_ec_reset(self.usb_dev)
