# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Dagwood zmake project configuration."""

register_raw_project(
    project_name="dagwood",
    zephyr_board="dagwood",
    modules=["hal_stm32", "cmsis_6"],
    supported_toolchains=["coreboot-sdk", "zephyr"],
)
