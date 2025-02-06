#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys
import time

import usb


args = None
target = None


def find_usb_device():
    dev = usb.core.find(manufacturer="Google", product="Dagwood")
    if dev is None:
        raise ValueError("Device not found")

    return dev


# bRequest values:
# bit 7 = 0: host to device
# bit 6..5 = vendor
# bit 4..0 = device
TO_DEV_REQ_TYPE = 0x40
# bit 7 = 0: device to host
# bit 6..5 = vendor
# bit 4..0 = device
TO_HOST_REQ_TYPE = 0xC0

# Must match the definitions in firmware/src/usb_request.h
USB_REQ_EC_RESET = 0
USB_REQ_POWER = 1
USB_REQ_NPCX_BOOT = 2


def main(argv):
    global args
    global target

    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", type=str, default="ttyACM2")
    args = parser.parse_args(argv)

    dev = find_usb_device()

    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 1, USB_REQ_POWER, 0)

    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 0, USB_REQ_NPCX_BOOT, 0)

    os.system(
        f"uartupdatetool --port={args.port} --baudrate=115200 --opr=wr --addr=0x200C3020 --file build/zephyr/npcx-aic/output/npcx_monitor.bin"
    )
    os.system(
        f"uartupdatetool --port={args.port} --baudrate=115200 --opr=wr --auto --offset=0 --file build/zephyr/npcx-aic/output/ec.bin"
    )

    dev.ctrl_transfer(TO_DEV_REQ_TYPE, 0, 0, USB_REQ_EC_RESET, 0)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
