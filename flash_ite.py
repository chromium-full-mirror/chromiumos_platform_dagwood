#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Flash an ITE EC image using the dagwood tester.

import argparse
import os
from pathlib import Path
import sys

import utils


args = None
target = None


def main(argv):
    global args
    global target

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-p", "--port", type=str, default=utils.find_default_ec_port()
    )
    parser.add_argument("-b", "--board", type=str, required=True)
    args = parser.parse_args(argv)

    board_path = Path("build", "zephyr", args.board)
    bin_file = board_path / "output" / "ec.bin"

    print(f"Programming EC binary: {bin_file}")

    dev = utils.find_usb_device()

    utils.dw_req_power(dev, True)

    # Use the -n "no_verify" flag by default reads are very slow using
    # the ITEFLASH tool.
    os.system(f"{utils.ITEFLASH} -d {args.port} -f {bin_file} -n")

    utils.dw_ec_reset(dev)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
