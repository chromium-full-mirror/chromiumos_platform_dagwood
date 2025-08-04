#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Flash an image to the AIC board using Dagwood"""

import os
from pathlib import Path
import subprocess
import sys

import yaml

import utils


args = None


class Platform:
    """Base class for a AIC platform"""

    NAME = "unknown"

    def __init__(self, build_path, usb_dev):
        self.build_path = build_path
        self.usb_dev = usb_dev
        self.port = utils.find_programming_port(usb_dev)

        self.bin_file = utils.verify_file(build_path / "output" / "ec.bin")

    @staticmethod
    def match(board_name):
        """Returns true if the class can handle board_name"""
        return False

    def enter_bootloader(self):
        """Enter bootloader for the platform"""
        pass

    def flash_commands(self):
        """Sequence of commands to run for flashing the image"""
        return []

    def exit_bootloader(self):
        """Leave bootloader for the platform"""
        pass


class PlatformNPCX(Platform):
    """NPCX platform class"""

    NAME = "NPCX"
    SDK_MONITOR_PATH = Path("/usr/share/ec-devutils/npcx_monitor.bin")
    UUT = "uartupdatetool"

    def __init__(self, build_path, usb_dev):
        super().__init__(build_path, usb_dev)

        self.monitor_file = utils.verify_file(
            build_path / "build-singleimage" / "npcx_monitor.bin",
            self.SDK_MONITOR_PATH,
        )

        self.port_base = os.path.basename(self.port)

        if args.run:
            elf_file = utils.verify_file(
                build_path / "build-ro" / "zephyr" / "zephyr.elf",
                build_path / "build-rw" / "zephyr" / "zephyr.elf",
                build_path / "build-singleimage" / "zephyr" / "zephyr.elf",
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

    def npcx_flash_commands(self):
        flash_monitor = [
            self.UUT,
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            "--addr=0x200C3020",
            f"--file={self.monitor_file}",
        ]

        flash_image = [
            self.UUT,
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            "--auto",
            "--offset=0",
            f"--file={self.bin_file}",
        ]
        return [flash_monitor, flash_image]

    def npcx_run_commands(self):
        load_image = [
            self.UUT,
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            f"--addr={self.rom_start}",
            f"--file={self.bin_file}",
        ]
        run_image = [
            self.UUT,
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=go",
            f"--addr={self.entry}",
        ]
        return [load_image, run_image]

    def flash_commands(self):

        if args.run:
            return self.npcx_run_commands()
        else:
            return self.npcx_flash_commands()

    def exit_bootloader(self):
        if not args.run:
            utils.dw_ec_reset(self.usb_dev)


class PlatformITE(Platform):
    """ITE platform class"""

    NAME = "ITE"
    ITEFLASH = "itecomdbgr"

    @staticmethod
    def match(board_name):
        return board_name in ["it8xxx2"]

    def flash_commands(self):
        # Use the -n "no_verify" flag by default reads are very slow using
        # the ITEFLASH tool.
        flash_image = [
            self.ITEFLASH,
            "-d",
            self.port,
            "-f",
            str(self.bin_file),
            "-n",
        ]

        return [flash_image]

    def exit_bootloader(self):
        utils.dw_ec_reset(self.usb_dev)


class PlatformRTS(Platform):
    """RTS platform class"""

    NAME = "RTS"
    RTKFLASH = "rtkupdate"

    def __init__(self, build_path, usb_dev):
        super().__init__(build_path, usb_dev)

        self.monitor_file = utils.verify_file(
            build_path / "output" / "rts5915_flash_upload.bin",
        )

    @staticmethod
    def match(board_name):
        return board_name in ["realtek"]

    def enter_bootloader(self):
        utils.dw_req_npcx_boot(self.usb_dev)

    def flash_commands(self):
        disable_wr = [
            self.RTKFLASH,
            "--method=wp",
            "--protect=0",
            f"--uart_device={self.port}",
        ]
        flash_monitor = [
            self.RTKFLASH,
            "--method=frame",
            f"--file={self.monitor_file}",
            f"--uart_device={self.port}",
        ]
        flash_image = [
            self.RTKFLASH,
            "--method=flash",
            "--spi_start=0x00000000",
            f"--file={self.bin_file}",
            f"--uart_device={self.port}",
        ]

        return [disable_wr, flash_monitor, flash_image]

    def exit_bootloader(self):
        utils.dw_ec_reset(self.usb_dev)


def load_build_info(build_path):
    """Find and returns a parsed build_info yaml file"""
    build_info_path = utils.verify_file(
        build_path / "build-ro" / "build_info.yml",
        build_path / "build-rw" / "build_info.yml",
        build_path / "build-singleimage" / "build_info.yml",
    )

    with open(build_info_path, "r") as f:
        build_info = yaml.safe_load(f)

    return build_info


def find_platform(board_name):
    """Find a Platform class matching board_name"""
    for platform in [PlatformNPCX, PlatformITE, PlatformRTS]:
        if platform.match(board_name):
            return platform

    print(f"No platform matching {board_name}")
    sys.exit(1)


def main(argv):
    global args
    args = utils.parse_arguments(argv)

    build_path = Path("build", "zephyr", args.board)

    build_info = load_build_info(build_path)
    board_name = build_info["cmake"]["board"]["name"]
    board_platform = find_platform(board_name)

    usb_dev = utils.find_usb_device(args.board_id)

    platform = board_platform(build_path, usb_dev)
    print(f"Board: {board_name} platform: {platform.NAME}")

    utils.dw_req_power(usb_dev, True)
    utils.dw_ec_uart_select_alt(usb_dev, True)

    platform.enter_bootloader()

    flash_commands = platform.flash_commands()
    for command in flash_commands:
        print(f"Running: \"{' '.join(command)}\"")

        subprocess.run(
            command,
            check=True,
            env=os.environ,
        )

    utils.dw_ec_uart_select_alt(usb_dev, False)

    platform.exit_bootloader()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
