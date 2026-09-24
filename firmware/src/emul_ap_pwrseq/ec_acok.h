/* Copyright 2026 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DAGWOOD_EMUL_AP_PWRSEQ_EC_ACOK_H_
#define DAGWOOD_EMUL_AP_PWRSEQ_EC_ACOK_H_

#include <stdbool.h>

/**
 * @brief Set the logical state of the EC ACOK output.
 *
 * @param enable true to assert ACOK (AC present), false to deassert.
 * @return 0 on success, negative errno on error.
 */
int ec_acok_set_state(bool enable);

/**
 * @brief Get the current logical state of the EC ACOK output.
 *
 * @return 1 if asserted (AC present), 0 if deasserted.
 */
int ec_acok_get_state(void);

#endif /* DAGWOOD_EMUL_AP_PWRSEQ_EC_ACOK_H_ */
