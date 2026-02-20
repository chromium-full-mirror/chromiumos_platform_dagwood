#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Flash an image to the AIC board using Dagwood"""

import os
from pathlib import Path
import subprocess
import sys

from platform_ite import PlatformITE
from platform_npcx import PlatformNPCX
from platform_rtk import PlatformRTK
import yaml

import utils


args = None


def load_build_info(build_path):
    """Find and returns a parsed build_info yaml file"""
    build_info_path = utils.verify_file(
        build_path / "build-ro" / "build_info.yml",
        build_path / "build-rw" / "build_info.yml",
        build_path / "build-singleimage" / "build_info.yml",
        build_path / "build_info.yml",
    )

    with open(build_info_path, "r") as f:
        build_info = yaml.safe_load(f)

    return build_info


def find_platform(board_name):
    """Find a Platform class matching board_name"""
    for platform in [PlatformNPCX, PlatformITE, PlatformRTK]:
        if platform.match(board_name):
            return platform

    print(f"No platform matching {board_name}")
    sys.exit(1)


def main(argv):
    global args
    args = utils.parse_arguments(argv)

    usb_dev = utils.find_usb_device(args.board_id)

    if args.reboot:
        utils.dw_reboot(usb_dev)
        sys.exit(0)

    if not args.board and not args.build_dir:
        print(f"Either --board or --build-dir has to be specified")
        sys.exit(1)

    if args.board and args.build_dir:
        print(f"Board and build-dir cannot be specified at the same time")
        sys.exit(1)

    if args.board:
        build_path = Path("build", "zephyr", args.board)
    else:
        build_path = args.build_dir

    build_info = load_build_info(build_path)
    board_name = build_info["cmake"]["board"]["name"]
    board_platform = find_platform(board_name)

    platform = board_platform(build_path, usb_dev, args)
    print(f"Board: {board_name} platform: {platform.NAME}")

    utils.dw_req_power(usb_dev, True)
    utils.dw_ec_uart_select_alt(usb_dev, True)

    platform.enter_bootloader()

    platform.preserve_sections()

    flash_commands = platform.flash_commands()
    try:
        for command in flash_commands:
            subprocess.run(
                command,
                check=True,
                env=os.environ,
            )
    finally:
        if platform.temp_bin and os.path.exists(platform.bin_file):
            os.remove(platform.bin_file)

    utils.dw_ec_uart_select_alt(usb_dev, False)

    platform.exit_bootloader()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
