# Copyright 2026 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

target_sources_ifdef(CONFIG_DT_HAS_CROS_EC_PWRBTN_ENABLED app PRIVATE ec_pwrbtn.c)
target_sources_ifdef(CONFIG_DT_HAS_CROS_EC_ACOK_ENABLED app PRIVATE ec_acok.c)
target_sources_ifdef(CONFIG_DT_HAS_CROS_X86_MTL_EMULATOR_ENABLED app PRIVATE x86_mtl_emulator.c)
