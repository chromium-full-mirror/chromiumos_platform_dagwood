#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Flash an image to the AIC board using Dagwood"""

import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

import yaml

import utils


args = None


class Platform:
    """Base class for a AIC platform"""

    NAME = "unknown"
    ZEPHYR_BIN_FILE = "zephyr.bin"

    def __init__(self, build_path, usb_dev):
        self.build_path = build_path
        self.usb_dev = usb_dev
        self.port = utils.find_programming_port(usb_dev)

        paths = [
            build_path / "output" / "ec.bin",
            build_path / "zephyr" / self.ZEPHYR_BIN_FILE,
        ]

        if args.ro:
            paths.insert(0, build_path / "packer" / "zephyr_ro.bin")

        self.bin_file = utils.verify_file(*paths)

        self.temp_bin = False
        self.dump_fmap = utils.verify_executable("dump_fmap")
        self.futility = utils.verify_executable("futility")

    @staticmethod
    def match(board_name):
        """Returns true if the class can handle board_name"""
        return False

    def read_partially(self, offset, size, file_path):
        """Read a portion of the flash"""
        raise NotImplementedError()

    def read_chip(self, file_path):
        """Read the entire flash"""
        raise NotImplementedError()

    def preserve_sections(self):
        """Preserve sections of the image that are marked with the 'preserve' flag"""

        # Nothing to preserve if loading into RAM.
        if args.no_preserve or args.run:
            return

        print("Checking for sections to preserve...")

        # Create a copy of the bin file to modify
        modified_bin = self.bin_file.parent / (self.bin_file.name + ".tmp")
        shutil.copy(self.bin_file, modified_bin)
        preservation_performed = False

        try:
            with tempfile.TemporaryDirectory() as temp_dir:
                temp_path = Path(temp_dir)

                # 1. Get FMAP location from new image
                new_image_fmap = self._read_fmap_header(modified_bin)

                if new_image_fmap is None:
                    print("No FMAP found in new image, skipping preservation.")
                    os.remove(modified_bin)
                    return

                if fmap_entry := new_image_fmap.get("FMAP"):
                    fmap_offset = fmap_entry["offset"]
                    fmap_size = fmap_entry["size"]
                else:
                    print("No FMAP found in new image, skipping preservation.")
                    return

                # 2. Read FMAP from chip
                cur_fmap_img = temp_path / "ec.cur.fmap.bin"
                try:
                    self.read_partially(fmap_offset, fmap_size, cur_fmap_img)
                except Exception as e:
                    print(f"Failed to read FMAP from chip: {e}")
                    os.remove(modified_bin)
                    return

                # 3. Verify FMAP header
                cur_ec_fmap = self._read_fmap_header(cur_fmap_img)
                if cur_ec_fmap is None:
                    print(
                        "FMAP header not found at expected offset reading entire chip..."
                    )

                    cur_img = temp_path / "ec.cur.bin"
                    try:
                        self.read_chip(cur_img)
                    except Exception as e:
                        print(f"Failed to read chip: {e}")
                        os.remove(modified_bin)
                        return

                    cur_ec_fmap = self._read_fmap_header(cur_img)
                    if cur_ec_fmap is None:
                        print("No FMAP found in chip, skipping preservation.")
                        os.remove(modified_bin)
                        return

                # 4. Extract preserved sections using FMAP
                preservation_performed = self._preserve_from_fmap(
                    cur_ec_fmap, new_image_fmap, modified_bin, temp_path
                )

        except Exception as e:
            print(f"Error during preservation: {e}")
            if os.path.exists(modified_bin):
                os.remove(modified_bin)
            return

        if preservation_performed:
            self.bin_file = modified_bin
            self.temp_bin = True
        else:
            os.remove(modified_bin)

    def _read_fmap_header(self, fmap_file):
        # Read the FMAP from a full EC binary or binary fragment
        try:
            cmd = [self.dump_fmap, "-e", str(fmap_file)]
            print(f"FMAP cmd: {' '.join(cmd)}")
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=True,
            )

            fmap = {}

            for line in result.stdout.strip().split("\n"):
                # Split by whitespace: [name, offset, size, flag]
                parts = line.split()
                if len(parts) == 4:
                    name, offset, size, flag = parts
                    fmap[name] = {
                        "offset": int(offset),
                        "size": int(size),
                        "preserve": flag
                        == "preserve",  # Converts to boolean True/False
                    }
        except subprocess.CalledProcessError:
            print("No FMAP found in new image, skipping preservation.")
            return None

        return fmap

    def _preserve_from_fmap(
        self, cur_ec_fmap, new_img_fmap, new_img, temp_path
    ):
        for name, fmap_entry in cur_ec_fmap.items():
            if fmap_entry["preserve"]:
                offset = fmap_entry["offset"]
                size = fmap_entry["size"]

                # Check if section exists in new image
                if name not in new_img_fmap:
                    print(f"Section {name} not found in new image, skipping.")
                    continue

                sec_img = temp_path / f"ec.cur.{name}.bin"
                print(f"Preserving {name} section...")
                self.read_partially(offset, size, sec_img)
                subprocess.run(
                    [
                        self.futility,
                        "load_fmap",
                        str(new_img),
                        f"{name}:{sec_img}",
                    ],
                    check=True,
                )
                preservation_performed = True
        return preservation_performed

    def enter_bootloader(self):
        """Enter bootloader for the platform"""
        pass

    def flash_commands(self):
        """Sequence of commands to run for flashing the image"""
        return []

    def exit_bootloader(self):
        """Leave bootloader for the platform"""
        pass


class PlatformNPCX(Platform):
    """NPCX platform class"""

    NAME = "NPCX"
    SDK_MONITOR_PATH = Path("/usr/share/ec-devutils/npcx_monitor.bin")
    UUT = "uartupdatetool"
    ZEPHYR_BIN_FILE = "zephyr.npcx.bin"

    def __init__(self, build_path, usb_dev):
        super().__init__(build_path, usb_dev)

        self.monitor_file = utils.verify_file(
            build_path / "build-singleimage" / "npcx_monitor.bin",
            self.SDK_MONITOR_PATH,
        )

        self.uut = utils.verify_executable(
            Path("build") / "host" / "util" / self.UUT,
            self.UUT,
        )

        self.port_base = os.path.basename(self.port)

        if args.run:
            elf_file = utils.verify_file(
                build_path / "build-ro" / "zephyr" / "zephyr.elf",
                build_path / "build-rw" / "zephyr" / "zephyr.elf",
                build_path / "build-singleimage" / "zephyr" / "zephyr.elf",
                build_path / "zephyr" / "zephyr.elf",
            )
            self.bin_file = elf_file.with_suffix(".bin")
            self.entry, self.rom_start = utils.elf_offsets(elf_file)
            print(
                f"File: {elf_file} entry point: {self.entry} ROM start: {self.rom_start}"
            )

    @staticmethod
    def match(board_name):
        return board_name in ["npcx9", "npcx7"]

    def enter_bootloader(self):
        utils.dw_req_npcx_boot(self.usb_dev)

    def npcx_flash_commands(self):
        flash_monitor = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            "--addr=0x200C3020",
            f"--file={self.monitor_file}",
        ]

        flash_image = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            "--auto",
            "--offset=0",
            f"--file={self.bin_file}",
        ]
        return [flash_monitor, flash_image]

    def npcx_run_commands(self):
        load_image = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=wr",
            f"--addr={self.rom_start}",
            f"--file={self.bin_file}",
        ]
        run_image = [
            str(self.uut),
            f"--port={self.port_base}",
            "--baudrate=115200",
            "--opr=go",
            f"--addr={self.entry}",
        ]
        return [load_image, run_image]

    def flash_commands(self):

        if args.run:
            return self.npcx_run_commands()
        else:
            return self.npcx_flash_commands()

    def exit_bootloader(self):
        if not args.run:
            utils.dw_ec_reset(self.usb_dev)


class PlatformITE(Platform):
    """ITE platform class"""

    NAME = "ITE"
    ITEFLASH = "itecomdbgr"

    def __init__(self, build_path, usb_dev):
        super().__init__(build_path, usb_dev)

        self.iteflash = utils.verify_executable(
            Path("build") / "host" / "util" / self.ITEFLASH,
            self.ITEFLASH,
        )

    @staticmethod
    def match(board_name):
        return board_name in ["it8xxx2"]

    def flash_commands(self):
        # Use the -n "no_verify" flag by default reads are very slow using
        # the ITEFLASH tool.
        flash_image = [
            str(self.iteflash),
            "-d",
            self.port,
            "-f",
            str(self.bin_file),
            "-n",
        ]

        return [flash_image]

    def exit_bootloader(self):
        utils.dw_ec_reset(self.usb_dev)


class PlatformRTS(Platform):
    """RTS platform class"""

    NAME = "RTS"
    RTKFLASH = "rtkupdate"
    ZEPHYR_BIN_FILE = "zephyr.rts5912.bin"

    def __init__(self, build_path, usb_dev):
        super().__init__(build_path, usb_dev)

        self.monitor_file = utils.verify_file(
            build_path / "output" / "rts5915_flash_upload.bin",
            build_path / "rts5915_flash_upload.bin",
        )

        self.rtkflash = utils.verify_executable(
            Path("build") / "host" / "util" / self.RTKFLASH,
            self.RTKFLASH,
        )

    @staticmethod
    def match(board_name):
        return board_name in ["realtek"]

    def enter_bootloader(self):
        utils.dw_req_npcx_boot(self.usb_dev)
        disable_wr = [
            str(self.rtkflash),
            "--method=wp",
            "--protect=0",
            f"--uart_device={self.port}",
        ]
        subprocess.run(disable_wr, check=True)
        flash_monitor = [
            str(self.rtkflash),
            "--method=frame",
            f"--file={self.monitor_file}",
            f"--uart_device={self.port}",
        ]
        subprocess.run(flash_monitor, check=True)

    def read_partially(self, offset, size, file_path):
        command = [
            str(self.rtkflash),
            "--method=read_bin",
            f"--spi_start={offset}",
            f"--file={file_path}",
            f"--bin_length={size}",
            f"--uart_device={self.port}",
        ]
        subprocess.run(command, check=True)

    def read_chip(self, file_path):
        command = [
            str(self.rtkflash),
            "--method=read_bin",
            "--spi_start=0x0",
            f"--file={file_path}",
            "--bin_length=0x100000",
            f"--uart_device={self.port}",
        ]
        subprocess.run(command, check=True)

    def flash_commands(self):
        flash_image = [
            str(self.rtkflash),
            "--method=flash",
            "--spi_start=0x00000000",
            f"--file={self.bin_file}",
            f"--uart_device={self.port}",
        ]

        return [flash_image]

    def exit_bootloader(self):
        utils.dw_ec_reset(self.usb_dev)


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
    for platform in [PlatformNPCX, PlatformITE, PlatformRTS]:
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

    platform = board_platform(build_path, usb_dev)
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
