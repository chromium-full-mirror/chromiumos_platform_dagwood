# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from pathlib import Path
import subprocess

from platform_base import Platform

import utils


class PlatformITE(Platform):
    """ITE platform class"""

    NAME = "ITE"
    ITEFLASH = "itecomdbgr"

    def __init__(self, build_path, usb_dev, args):
        super().__init__(build_path, usb_dev, args)

        self.iteflash = utils.verify_executable(
            Path("build") / "host" / "util" / self.ITEFLASH,
            self.ITEFLASH,
        )

    @staticmethod
    def match(board_name):
        return board_name in ["it8xxx2"]

    def read_partially(self, offset, size, file_path):
        read_command = [
            str(self.iteflash),
            "-d",
            self.port,
            "-r",
            str(file_path),
            "-R",
            str(offset),
            str(size),
        ]
        subprocess.run(read_command, check=True)

    def read_chip(self, file_path):
        read_command = [
            str(self.iteflash),
            "-d",
            self.port,
            "-r",
            str(file_path),
        ]
        subprocess.run(read_command, check=True)

    def flash_commands(self):
        # Use the -n "no_verify" flag by default reads are very slow using
        # the ITEFLASH tool.
        flash_image = [
            str(self.iteflash),
            "-d",
            self.port,
            "-f",
            str(self.bin_file),
            "-n",
        ]

        return [flash_image]

    def exit_bootloader(self):
        utils.dw_ec_reset(self.usb_dev)
