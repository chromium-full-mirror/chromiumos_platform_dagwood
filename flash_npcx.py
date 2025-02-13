#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys
import time

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
    args = parser.parse_args(argv)

    dev = utils.find_usb_device()

    utils.dw_req_power(dev, True)
    utils.dw_req_npcx_boot(dev)

    port_base = os.path.basename(args.port)

    os.system(
        f"{utils.UUT} --port={port_base} --baudrate=115200 --opr=wr --addr=0x200C3020 --file build/zephyr/npcx-aic/output/npcx_monitor.bin"
    )
    os.system(
        f"{utils.UUT} --port={port_base} --baudrate=115200 --opr=wr --auto --offset=0 --file build/zephyr/npcx-aic/output/ec.bin"
    )

    utils.dw_ec_reset(dev)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
