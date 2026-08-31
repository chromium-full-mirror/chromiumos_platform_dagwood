# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD dagwood)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE} ${ZEPHYR_BASE})
project(dagwood)

if(NOT DEFINED PLATFORM_EC)
	if(DEFINED ENV{PLATFORM_EC})
		set(PLATFORM_EC "$ENV{PLATFORM_EC}")
	else()
		set(PLATFORM_EC "${CMAKE_CURRENT_SOURCE_DIR}/../../ec")
	endif()
endif()

separate_arguments(EXTRA_EC_VERSION_FLAGS_LIST UNIX_COMMAND
		   ${EXTRA_EC_VERSION_FLAGS})

execute_process(
	COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${PLATFORM_EC}/zephyr/zmake/
		${PYTHON_EXECUTABLE}
		"${PLATFORM_EC}/zephyr/zmake/zephyr_build_tools/generate_ec_version.py"
		"-"
		"--name" "dagwood"
		${EXTRA_EC_VERSION_FLAGS_LIST}
	OUTPUT_VARIABLE DW_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET
)

if(NOT DW_VERSION)
	set(DW_VERSION "dagwood-unknown")
endif()

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
	${ZEPHYR_BINARY_DIR}/include/generated/dagwood/version.h
	@ONLY
)

zephyr_include_directories(include)

target_sources(app PRIVATE
	src/ec.c
	src/main.c
	src/reboot.c
	src/uart_select.c
	src/usbd.c
	src/version.c
)

target_sources_ifdef(CONFIG_DT_HAS_CROS_NPCX_BOOT_ENABLED app PRIVATE src/npcx_boot.c)
target_sources_ifdef(CONFIG_DT_HAS_CROS_I2C_TARGET_MEMORY_ENABLED app PRIVATE src/i2c_target.c)
target_sources_ifdef(CONFIG_DT_HAS_CROS_I2C_MUX_ENABLED app PRIVATE src/i2c_mux.c)
target_sources_ifdef(CONFIG_DT_HAS_CROS_FAKE_FAN_ENABLED app PRIVATE src/fake_fan.c)

zephyr_linker_sources(ROM_SECTIONS src/usb_request.ld)
