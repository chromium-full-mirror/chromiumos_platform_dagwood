#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Flash an ITE EC image using the dagwood tester.

import os
from pathlib import Path
import subprocess
import sys

import utils


args = None


def main(argv):
    global args
    args = utils.parse_arguments(argv)

    board_path = Path("build", "zephyr", args.board)
    bin_file = board_path / "output" / "ec.bin"

    utils.verify_file(bin_file)

    print(f"Programming EC binary: {bin_file}")

    dev = utils.find_usb_device(args.board_id)
    port = utils.find_programming_port(dev)

    utils.dw_req_power(dev, True)
    utils.dw_ec_uart_select_alt(dev, True)

    # Use the -n "no_verify" flag by default reads are very slow using
    # the ITEFLASH tool.
    subprocess.run(
        [utils.ITEFLASH, "-d", port, "-f", bin_file, "-n"],
        check=True,
        env=os.environ,
    )

    utils.dw_ec_uart_select_alt(dev, False)
    utils.dw_ec_reset(dev)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
