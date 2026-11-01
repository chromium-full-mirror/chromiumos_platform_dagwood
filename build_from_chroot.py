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
import time

import dagwood_utils
import usb.core


SELF_PATH = Path(__file__).parent.resolve()
DFU_UTIL = "dfu-util"
DAGWOOD_USB_VID = 0x18D1
DAGWOOD_USB_PID = 0x5214
DFU_USB_VID = 0x0483
DFU_USB_PID = 0xDF11
DEVICE_USB_ID = f"{DFU_USB_VID:04x}:{DFU_USB_PID:04x}"
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


def is_usb_device_present(vid, pid, serial=None):
    """Check if a USB device with the given VID:PID (and serial) is present."""
    kwargs = {"idVendor": vid, "idProduct": pid}
    if serial is not None:
        kwargs["serial_number"] = serial
    return usb.core.find(**kwargs) is not None


def enter_dfu_if_needed(board_id=None, timeout_s=5.0):
    """Ensure the Dagwood board is in STM32 DFU mode, triggering it via USB."""
    # Note: this matches any connected STM32 device in DFU mode since the ROM
    # bootloader DFU descriptor cannot be filtered by Dagwood board serial.
    if is_usb_device_present(DFU_USB_VID, DFU_USB_PID):
        print(f"Found device already in STM32 DFU mode ({DEVICE_USB_ID}).")
        return

    try:
        usb_dev = dagwood_utils.find_usb_device(board_id)
    except (ValueError, usb.core.USBError) as e:
        raise RuntimeError(
            f"Neither a running Dagwood device nor an STM32 DFU device "
            f"({DEVICE_USB_ID}) was found ({e})."
        ) from e

    print("Requesting Dagwood to reboot into STM32 DFU mode...")
    dagwood_utils.dw_dfu(usb_dev)

    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        if is_usb_device_present(DFU_USB_VID, DFU_USB_PID):
            time.sleep(0.2)
            return
        time.sleep(0.1)

    raise RuntimeError(
        f"Timed out waiting for Dagwood to enter DFU mode ({DEVICE_USB_ID})."
    )


def wait_for_dagwood_boot(board_id=None, timeout_s=5.0):
    """Wait for Dagwood to re-enumerate in normal application mode after DFU."""
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        if is_usb_device_present(
            DAGWOOD_USB_VID, DAGWOOD_USB_PID, serial=board_id
        ):
            return True
        time.sleep(0.2)
    return False


def flash_dfu(build_path, board_id=None):
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

    enter_dfu_if_needed(board_id)

    dfu_cmd = [
        DFU_UTIL,
        f"-d,{DEVICE_USB_ID}",
        "-s",
        "0x8000000:leave",
        "-a",
        "0",
        "-D",
        str(bin_path),
    ]
    proc = subprocess.run(
        dfu_cmd,
        check=False,
        env=os.environ,
    )
    # The STM32H7 ROM bootloader jumps to the application during the final
    # DFU_GETSTATUS poll on ":leave", which can cause dfu-util to exit with 74
    # (EX_IOERR) even though manifestation succeeded.
    if proc.returncode not in (0, 74):
        raise subprocess.CalledProcessError(proc.returncode, dfu_cmd)

    if not wait_for_dagwood_boot(board_id):
        if proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, dfu_cmd)
        print(
            "Warning: Timed out waiting for Dagwood USB device to re-enumerate."
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
                flash_dfu(build_path, args.board_id)
        except RuntimeError as e:
            print(f"Error: {e}", file=sys.stderr)
            return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
