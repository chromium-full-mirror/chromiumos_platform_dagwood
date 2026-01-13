#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build and flash the Dagwood firmware from inside a ChromiumOS SDK chroot

This builds and optionally flash a Dagwood firmware from a ChromiumOS SDK
chroot, using zmake for setting up the toolchain, configuring and starting the
build.

Flashing is only supported using dfu-util.
"""

import argparse
import multiprocessing
import os
from pathlib import Path
import subprocess
import sys


SELF_PATH = Path(__file__).parent.resolve()
DFU_UTIL = "dfu-util"
DEVICE_USB_ID = "0483:df11"
PROJECT_NAME = "dagwood"

args = None

# Valid log level strings, should match zmake -l option
log_levels = ["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"]


def main(argv):
    global args

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output-dir",
        required=False,
        help="Full pathanme for the directory in which to bundle build artifacts.",
    )
    parser.add_argument(
        "-f",
        "--flash",
        action="store_true",
        help="Run dfu-util to flash the firmware after the build",
    )
    parser.add_argument(
        "--integration",
        action="store_true",
        help="Tweak few zmake flags for the CI integration run",
    )

    parser.add_argument(
        "-l",
        "--log-level",
        choices=log_levels,
        default="INFO",
        help="Set the logging level (default=INFO)",
    )

    parser.add_argument(
        "--clobber",
        action="store_true",
        dest="clobber",
        help="Delete existing build directories, even if configuration is unchanged",
    )

    args = parser.parse_args(argv)

    if not args.output_dir:
        build_path = SELF_PATH / "build"
    else:
        build_path = args.output_dir

    zmake_cmd = ["zmake", "--projects-dir", str(SELF_PATH)]
    zmake_cmd.append("--log-level")
    zmake_cmd.append(args.log_level.upper())

    if args.integration:
        zmake_cmd.append("-D")

    zmake_cmd.extend(["build", "-B", str(build_path), PROJECT_NAME])

    if args.clobber:
        zmake_cmd.append("--clobber")

    subprocess.run(
        zmake_cmd,
        check=True,
        env=os.environ,
    )

    if args.flash:
        bin_path = (
            build_path
            / "dagwood"
            / "build-singleimage"
            / "zephyr"
            / "zephyr.bin"
        )
        subprocess.run(
            [
                # fmt: off
                    DFU_UTIL,
                    f"-d,{DEVICE_USB_ID}",
                    "-s", "0x8000000:leave",
                    "-a", "0",
                    "-D", str(bin_path),
                # fmt: on
            ],
            check=True,
            env=os.environ,
        )


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
