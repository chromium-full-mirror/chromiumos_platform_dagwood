# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
from pathlib import Path
import shutil
import subprocess
import tempfile

import utils


class Platform:
    """Base class for a AIC platform"""

    NAME = "unknown"
    ZEPHYR_BIN_FILE = "zephyr.bin"

    def __init__(self, build_path, usb_dev, args):
        self.build_path = build_path
        self.usb_dev = usb_dev
        self.args = args
        self.port = utils.find_programming_port(usb_dev)

        paths = [
            build_path / "output" / "ec.bin",
            build_path / "zephyr" / self.ZEPHYR_BIN_FILE,
        ]

        if self.args.ro:
            paths.insert(0, build_path / "packer" / "zephyr_ro.bin")

        self.bin_file = utils.verify_file(*paths)

        self.temp_bin = False

        if not self.args.no_preserve:
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
        if self.args.no_preserve or self.args.run:
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
            print(f"No FMAP found in {fmap_file}")
            return None

        return fmap

    def _preserve_from_fmap(
        self, cur_ec_fmap, new_img_fmap, new_img, temp_path
    ):
        preservation_performed = False
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
