# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from pathlib import Path
import subprocess

from platform_base import Platform

import utils


class PlatformRTK(Platform):
    """RTK platform class"""

    NAME = "RTK"
    RTKFLASH = "rtkupdate"
    ZEPHYR_BIN_FILE = "zephyr.rts5912.bin"
    SDK_MOTNITOR_PATH = Path("/usr/share/ec-devutils/rts5915_flash_upload.bin")

    def __init__(self, build_path, usb_dev, args):
        super().__init__(build_path, usb_dev, args)

        self.monitor_file = utils.verify_file(
            build_path.parent
            / "rtk_flame"
            / "build-singleimage"
            / "rts5915_flash_upload.bin",
            self.SDK_MOTNITOR_PATH,
        )

        # b/476201042, always power off the EC at the start of programming.
        # This ensures we don't trigger the deep sleep hang.
        print("RTK detected: powering off EC")
        utils.dw_req_power(usb_dev, False)

        self.rtkflash = utils.verify_executable(
            Path("build") / "host" / "util" / self.RTKFLASH,
            self.RTKFLASH,
        )

        if self.args.run:
            elf_file = utils.verify_file(
                build_path / "build-ro" / "zephyr" / "zephyr.elf",
                build_path / "build-rw" / "zephyr" / "zephyr.elf",
                build_path / "build-singleimage" / "zephyr" / "zephyr.elf",
                build_path / "zephyr" / "zephyr.elf",
            )
            self.bin_file = elf_file.with_suffix(".bin")
            self.entry, self.rom_start = utils.elf_offsets(elf_file)

            # b/518863832: The Realtek bootrom only supports jumping to address
            # 0x20010020. Only support images that load to this address and
            # also have the entry point set to the ARM/THUMB jump address
            # of 0x20010020.
            if self.rom_start != "0x20010020":
                raise ValueError(
                    f"Invalid ROM start address {self.rom_start}, expected 0x20010020"
                )
            if self.entry not in ["0x20010020", "0x20010021"]:
                raise ValueError(
                    f"Invalid entry point {self.entry}, expected 0x20010020 or 0x20010021"
                )

            print(
                f"File: {self.bin_file} entry point: {self.entry} ROM start: {self.rom_start}"
            )

    @staticmethod
    def match(board_name):
        return board_name in ["realtek"]

    def enter_bootloader(self):
        utils.dw_req_npcx_boot(self.usb_dev)
        disable_wr = [
            str(self.rtkflash),
            "--method=wp",
            "--protect=0",
            f"--uart_device={self.port}",
        ]
        subprocess.run(disable_wr, check=True)

        if self.args.run:
            return

        flash_monitor = [
            str(self.rtkflash),
            "--method=frame",
            f"--file={self.monitor_file}",
            f"--uart_device={self.port}",
        ]
        subprocess.run(flash_monitor, check=True)

    def read_partially(self, offset, size, file_path):
        command = [
            str(self.rtkflash),
            "--method=read_bin",
            f"--spi_start={offset}",
            f"--file={file_path}",
            f"--bin_length={size}",
            f"--uart_device={self.port}",
        ]
        subprocess.run(command, check=True)

    def read_chip(self, file_path):
        command = [
            str(self.rtkflash),
            "--method=read_bin",
            "--spi_start=0x0",
            f"--file={file_path}",
            "--bin_length=0x100000",
            f"--uart_device={self.port}",
        ]
        subprocess.run(command, check=True)

    def rtk_flash_command(self):
        flash_image = [
            str(self.rtkflash),
            "--method=flash",
            "--spi_start=0x00000000",
            f"--file={self.bin_file}",
            f"--uart_device={self.port}",
        ]
        return [flash_image]

    def rtk_run_command(self):
        command = [
            str(self.rtkflash),
            "--method=load_sram",
            f"--file={self.bin_file}",
            f"--uart_device={self.port}",
        ]
        return [command]

    def flash_commands(self):
        if self.args.run:
            return self.rtk_run_command()
        else:
            return self.rtk_flash_command()

    def exit_bootloader(self):
        if not self.args.run:
            utils.dw_ec_reset(self.usb_dev)
