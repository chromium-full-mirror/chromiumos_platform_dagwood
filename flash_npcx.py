#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
from pathlib import Path
import subprocess
import sys

import utils


args = None


SDK_MONITOR_PATH = Path("/usr/share/ec-devutils/npcx_monitor.bin")


def main(argv):
    global args
    args = utils.parse_arguments(argv)

    board_path = Path("build", "zephyr", args.board)
    bin_file = board_path / "output" / "ec.bin"
    monitor_file = board_path / "build-singleimage" / "npcx_monitor.bin"

    utils.verify_file(bin_file)
    monitor_file = utils.verify_file(monitor_file, SDK_MONITOR_PATH)

    print(f"Programming EC monitor={monitor_file} binary={bin_file}")

    dev = utils.find_usb_device(args.board_id)

    utils.dw_req_power(dev, True)
    utils.dw_ec_uart_select_alt(dev, True)
    utils.dw_req_npcx_boot(dev)

    port_base = os.path.basename(utils.find_programming_port(dev))

    subprocess.run(
        [
            utils.UUT,
            f"--port={port_base}",
            "--baudrate=115200",
            "--opr=wr",
            "--addr=0x200C3020",
            f"--file={monitor_file}",
        ],
        check=True,
        env=os.environ,
    )
    subprocess.run(
        [
            utils.UUT,
            f"--port={port_base}",
            "--baudrate=115200",
            "--opr=wr",
            "--auto",
            "--offset=0",
            f"--file={bin_file}",
        ],
        check=True,
        env=os.environ,
    )

    utils.dw_ec_uart_select_alt(dev, False)
    utils.dw_ec_reset(dev)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
