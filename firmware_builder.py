#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build the Dagwood project firmware

Build the Dagwood project firmware, this is meant to be called by the firmware
builders in CI.
"""

import argparse
import os
import pathlib
import site
import subprocess
import sys

from google.protobuf import json_format

from chromite.api.gen_sdk.chromite.api import firmware_pb2


SELF_PATH = pathlib.Path(__file__).parent.resolve()


# This script is only used in CI so it should be fine to assume the EC
# repository is going to be present as well.
site.addsitedir(SELF_PATH.parent / "ec")

import zephyr.scripts.firmware_builder_lib


BUILD_FROM_CHROOT_PATH = SELF_PATH / "build_from_chroot.py"
PROJECT_NAME = "dagwood"


def build(opts):
    """Builds the Dagwood firmware"""

    metric_list = firmware_pb2.FwBuildMetricList()

    subprocess.run(
        [BUILD_FROM_CHROOT_PATH, "--integration"],
        check=True,
        env=os.environ,
    )

    with open(opts.metrics, "w", encoding="utf-8") as file:
        file.write(json_format.MessageToJson(metric_list))


def bundle(opts):
    """Bundle the artifacts."""
    info = firmware_pb2.FirmwareArtifactInfo()  # pylint: disable=no-member
    info.bcs_version_info.version_string = opts.bcs_version

    if not os.path.isdir(opts.output_dir):
        os.mkdir(opts.output_dir)

    artifacts_dir = SELF_PATH / "build" / PROJECT_NAME / "output"
    tarball_name = f"{PROJECT_NAME}.firmware.tar.bz2"
    tarball_path = (
        pathlib.Path(opts.output_dir).joinpath(tarball_name).resolve()
    )

    subprocess.run(
        ["tar", "jcvf", tarball_path, "."], check=True, cwd=artifacts_dir
    )

    meta = info.objects.add()
    meta.tarball_info.board.append(PROJECT_NAME)
    meta.file_name = tarball_name
    meta.tarball_info.type = (
        firmware_pb2.FirmwareArtifactInfo.TarballInfo.FirmwareType.EC
    )

    pathlib.Path(opts.metadata).write_text(
        json_format.MessageToJson(info), encoding="utf-8"
    )


def test(opts):
    """Runs all of the unit tests for Dagwood firmware"""

    # Nothing for now


def main(argv):
    """Builds or bundles the Dagwood firmware"""
    parser, _ = zephyr.scripts.firmware_builder_lib.create_arg_parser(
        build, bundle, test
    )
    opts = parser.parse_args(argv)

    if not hasattr(opts, "func"):
        print("Must select a valid sub command!")
        return -1

    # Run selected sub command function
    try:
        opts.func(opts)
    except subprocess.CalledProcessError:
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
