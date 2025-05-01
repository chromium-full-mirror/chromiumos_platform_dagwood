#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Program an RTS5915 AIC card through the dagwood tester
# Realtek ECs use the same mechanism as Nuvoton ECs to hold the chip in
# bootrom mode: pull the EC UART_TX pin when coming out of reset.
#

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

import utils


args = None
target = None


def main(argv):
    global args
    global target

    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--board", type=str, required=False)
    args = parser.parse_args(argv)

    board_path = Path("build", "zephyr", args.board)
    bin_file = board_path / "output" / "ec.bin"
    rtk_monitor_file = board_path / "output" / "rts5915_flash_upload.bin"

    dev = utils.find_usb_device()
    port = utils.find_programming_port(dev)

    utils.dw_req_power(dev, True)
    utils.dw_ec_uart_select_alt(dev, True)
    utils.dw_req_npcx_boot(dev)

    print("Disabling write protect")

    subprocess.run(
        [
            utils.RTKFLASH,
            "--method=wp",
            "--protect=0",
            f"--uart_device={port}",
        ],
        check=True,
        env=os.environ,
    )

    print("Loading RTK monitor")
    subprocess.run(
        [
            utils.RTKFLASH,
            "--method=frame",
            f"--file={rtk_monitor_file}",
            f"--uart_device={port}",
        ],
        check=True,
        env=os.environ,
    )

    print("Programming EC image")
    subprocess.run(
        [
            utils.RTKFLASH,
            "--method=flash",
            "--spi_start=0x00000000",
            f"--file={bin_file}",
            f"--uart_device={port}",
        ],
        check=True,
        env=os.environ,
    )

    utils.dw_ec_uart_select_alt(dev, False)
    utils.dw_ec_reset(dev)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
