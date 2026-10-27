#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build and flash the Dagwood firmware from inside a ChromiumOS SDK chroot / Docker

This builds and optionally flashes a Dagwood firmware from a ChromiumOS SDK
chroot or Docker container, using zmake for setting up the toolchain,
configuring and starting the build.

Flashing is supported using openocd (default) or dfu-util.
"""

import argparse
import multiprocessing
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


SELF_PATH = Path(__file__).parent.resolve()
DFU_UTIL = "dfu-util"
DEVICE_USB_ID = "0483:df11"
PROJECT_NAME = "dagwood"
OPENOCD_CFG = (
    SELF_PATH
    / "firmware"
    / "boards"
    / "google"
    / "dagwood"
    / "support"
    / "openocd-stlink.cfg"
)

args = None

# Valid log level strings, should match zmake -l option
log_levels = ["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"]


def build_firmware(build_path, args):
    """Build the firmware using zmake."""
    checkout = args.checkout
    if not checkout and Path("/workspace").is_dir():
        checkout = Path("/workspace")

    zmake_cmd = ["zmake"]
    if checkout:
        zmake_cmd.extend(["--checkout", str(checkout)])
    zmake_cmd.extend(["--projects-dir", str(SELF_PATH)])
    zmake_cmd.extend(["--log-level", args.log_level.upper()])

    if args.integration:
        zmake_cmd.append("-D")

    zmake_cmd.extend(["build", "-B", str(build_path), PROJECT_NAME])

    if args.clobber:
        zmake_cmd.append("--clobber")

    env = os.environ.copy()
    env["ZMAKE_WORKSPACE_PREPARED"] = "1"

    subprocess.run(
        zmake_cmd,
        check=True,
        env=env,
    )


def flash_openocd(build_path):
    """Flash the firmware using OpenOCD."""
    if not shutil.which("openocd"):
        raise RuntimeError(
            "openocd not found in PATH. OpenOCD is required for flashing. "
            "Please run inside the EC Docker container or install openocd."
        )

    elf_path = (
        build_path
        / PROJECT_NAME
        / "build-singleimage"
        / "zephyr"
        / "zephyr.elf"
    )
    if not elf_path.is_file():
        raise FileNotFoundError(f"Could not find firmware binary: {elf_path}")

    openocd_cmd = [
        "openocd",
        "-f",
        str(OPENOCD_CFG),
        "-c",
        f"program {elf_path} verify reset exit",
    ]
    print(f"Flashing Dagwood via OpenOCD & ST-Link: {' '.join(openocd_cmd)}")
    subprocess.run(
        openocd_cmd,
        check=True,
        env=os.environ,
    )


def flash_dfu(build_path):
    """Flash the firmware using dfu-util."""
    if not shutil.which(DFU_UTIL):
        raise RuntimeError(
            f"{DFU_UTIL} not found in PATH. Please install {DFU_UTIL}."
        )

    bin_path = (
        build_path
        / PROJECT_NAME
        / "build-singleimage"
        / "zephyr"
        / "zephyr.bin"
    )
    if not bin_path.is_file():
        raise FileNotFoundError(f"Could not find firmware binary: {bin_path}")

    subprocess.run(
        [
            DFU_UTIL,
            f"-d,{DEVICE_USB_ID}",
            "-s",
            "0x8000000:leave",
            "-a",
            "0",
            "-D",
            str(bin_path),
        ],
        check=True,
        env=os.environ,
    )


def get_built_version(build_path):
    """Read the target firmware version from the generated version.h header."""
    version_h = (
        build_path
        / PROJECT_NAME
        / "build-singleimage"
        / "zephyr"
        / "include"
        / "generated"
        / "dagwood"
        / "version.h"
    )
    if not version_h.is_file():
        return None

    content = version_h.read_text(encoding="utf-8")
    match = re.search(r'#define\s+DW_VERSION\s+"([^"]+)"', content)
    if match:
        return match.group(1)
    return None


def get_running_version(board_id=None):
    """Query the firmware version currently running on the Dagwood board."""
    # pylint: disable=import-outside-toplevel
    import dagwood_utils
    import usb.core

    try:
        usb_dev = dagwood_utils.find_usb_device(board_id)
        return dagwood_utils.dw_version(usb_dev)
    except (ValueError, usb.core.USBError) as e:
        print(
            f"Warning: Could not query running Dagwood firmware version ({e})."
        )
        return None


def should_flash(build_path, board_id=None, force=False):
    """Check running vs built firmware version and return True if flash should proceed."""
    built_version = get_built_version(build_path)
    running_version = get_running_version(board_id)

    print(
        f"Dagwood firmware version: running={running_version}, target={built_version}"
    )

    if (
        running_version
        and built_version
        and running_version != "unknown"
        and built_version != "dagwood-unknown"
        and running_version == built_version
    ):
        if not force:
            print(
                f"Dagwood is already running {running_version}; "
                "skipping flash (use --force to override)."
            )
            return False
        print(
            f"Dagwood is already running {running_version}; "
            "flashing anyway (--force)."
        )

    return True


def main(argv):
    global args

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output-dir",
        required=False,
        help="Full pathname for the directory in which to bundle build artifacts.",
    )
    parser.add_argument(
        "-c",
        "--checkout",
        type=Path,
        required=False,
        help="Path to ChromeOS checkout or workspace directory.",
    )
    parser.add_argument(
        "-f",
        "--flash",
        action="store_true",
        help="Flash the firmware after the build",
    )
    parser.add_argument(
        "--flash-only",
        action="store_true",
        help="Flash existing firmware without building",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Force flashing even if the running firmware version matches the build",
    )
    parser.add_argument(
        "--board-id",
        type=str,
        required=False,
        help="Dagwood board USB serial number",
    )
    parser.add_argument(
        "--flasher",
        choices=["openocd", "dfu"],
        default="openocd",
        help="Tool to use for flashing (default: openocd)",
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
        build_path = Path(args.output_dir)

    if not args.flash_only:
        build_firmware(build_path, args)

    if args.flash or args.flash_only:
        if not should_flash(build_path, args.board_id, args.force):
            return 0
        try:
            if args.flasher == "openocd":
                flash_openocd(build_path)
            elif args.flasher == "dfu":
                flash_dfu(build_path)
        except RuntimeError as e:
            print(f"Error: {e}", file=sys.stderr)
            return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
