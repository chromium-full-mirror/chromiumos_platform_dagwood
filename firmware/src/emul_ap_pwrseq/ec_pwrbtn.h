/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DAGWOOD_EMUL_AP_PWRSEQ_EC_PWRBTN_H_
#define DAGWOOD_EMUL_AP_PWRSEQ_EC_PWRBTN_H_

#include <stdint.h>

/**
 * @brief Press the EC power button for a specified duration.
 *
 * @param duration_ms Duration in milliseconds to hold EC_PWRBTN_L asserted.
 * @return 0 on success, negative errno on error.
 */
int ec_pwrbtn_press(uint32_t duration_ms);

/**
 * @brief Get the current logical state of the EC power button output.
 *
 * @return 1 if asserted (pressed), 0 if deasserted.
 */
int ec_pwrbtn_get_state(void);

#endif /* DAGWOOD_EMUL_AP_PWRSEQ_EC_PWRBTN_H_ */
