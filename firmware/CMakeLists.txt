# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD dagwood)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE} ${ZEPHYR_BASE})
project(dagwood)

zephyr_include_directories(include)

target_sources(app PRIVATE
	src/ec.c
	src/main.c
	src/reboot.c
	src/uart_select.c
	src/usbd.c
)

target_sources_ifdef(CONFIG_DT_HAS_CROS_NPCX_BOOT_ENABLED app PRIVATE src/npcx_boot.c)
target_sources_ifdef(CONFIG_I2C_EEPROM_TARGET app PRIVATE src/i2c_target.c)
target_sources_ifdef(CONFIG_DT_HAS_CROS_I2C_MUX_ENABLED app PRIVATE src/i2c_mux.c)

zephyr_linker_sources(ROM_SECTIONS src/usb_request.ld)
