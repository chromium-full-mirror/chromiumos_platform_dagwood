# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

board_runner_args(jlink "--device=STM32H743ZG" "--speed=4000")
board_runner_args(pyocd "--target=stm32h743zgtx")
board_runner_args(openocd --cmd-post-verify "reset halt")
board_runner_args(openocd --target-handle=_CHIPNAME.cpu0)
board_runner_args(stm32cubeprogrammer "--port=swd" "--reset-mode=hw")
board_runner_args(dfu-util "--pid=0483:df11" "--alt=0" "--dfuse")

include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)
include(${ZEPHYR_BASE}/boards/common/dfu-util.board.cmake)
