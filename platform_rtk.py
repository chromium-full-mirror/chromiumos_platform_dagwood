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

    def __init__(self, build_path, usb_dev, args):
        super().__init__(build_path, usb_dev, args)

        self.monitor_file = utils.verify_file(
            build_path / "output" / "rts5915_flash_upload.bin",
            build_path / "rts5915_flash_upload.bin",
        )

        self.rtkflash = utils.verify_executable(
            Path("build") / "host" / "util" / self.RTKFLASH,
            self.RTKFLASH,
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

    def flash_commands(self):
        flash_image = [
            str(self.rtkflash),
            "--method=flash",
            "--spi_start=0x00000000",
            f"--file={self.bin_file}",
            f"--uart_device={self.port}",
        ]

        return [flash_image]

    def exit_bootloader(self):
        utils.dw_ec_reset(self.usb_dev)
