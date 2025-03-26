#!/usr/bin/env python3
# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build the Dagwood project firmware

Build the Dagwood project firmware, this is meant to be called by the firmware
builders in CI.
"""

import os
from pathlib import Path
import subprocess


SELF_PATH = Path(__file__).parent.resolve()
BUILD_FROM_CHROOT_PATH = SELF_PATH / "build_from_chroot.py"

subprocess.run(
    [BUILD_FROM_CHROOT_PATH, "--integration"],
    check=True,
    env=os.environ,
)
