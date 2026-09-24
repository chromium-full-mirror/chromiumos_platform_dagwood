/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stddef.h>
#include <stdint.h>

#define USB_REQ_EC_RESET 0
#define USB_REQ_POWER 1
#define USB_REQ_NPCX_BOOT 2
#define USB_REQ_EC_UART_SELECT_ALT 3
#define USB_REQ_REBOOT 4
#define USB_REQ_FAN_GET_PWM 5
#define USB_REQ_FAN_SET_TACH 6
#define USB_REQ_IDENTIFY 7
#define USB_REQ_VERSION 8
#define USB_REQ_I2C_MUX_A 9
#define USB_REQ_I2C_MUX_B 10
#define USB_REQ_AP_GET_STATUS 11
#define USB_REQ_AP_SET_MODE 12
#define USB_REQ_AP_PWRBTN 13
#define USB_REQ_AP_SET_ACOK 14
#define USB_REQ_AP_SET_SIGNAL 15

#define USB_REQUEST_MAX_BUF_SIZE 128

enum ap_state {
	AP_STATE_G3 = 0,
	AP_STATE_S5 = 1,
	AP_STATE_S4 = 2,
	AP_STATE_S3 = 3,
	AP_STATE_S0 = 4,
};

enum ap_mode {
	AP_MODE_AUTO = 0,
	AP_MODE_MANUAL = 1,
};

/* Signal bit flags */
#define AP_SIG_EN_S5 BIT(0)
#define AP_SIG_RSMRST BIT(1)
#define AP_SIG_SOC_PWRBTN BIT(2)
#define AP_SIG_SYS_PWROK BIT(3)
#define AP_SIG_RSMRST_PWRGD BIT(4)
#define AP_SIG_ALL_SYS_PWRGD BIT(5)
#define AP_SIG_SLP_S5 BIT(6)
#define AP_SIG_SLP_S4 BIT(7)
#define AP_SIG_SLP_S3 BIT(8)
#define AP_SIG_SLP_S0 BIT(9)
#define AP_SIG_EC_PWRBTN BIT(10)
#define AP_SIG_ACOK BIT(11)
#define AP_SIG_PCH_PWROK BIT(12)

struct usb_ap_status {
	uint8_t state;
	uint8_t mode;
	uint16_t signals;
};

struct usb_fan_pwm {
	uint32_t frequency;
	uint32_t duty_cycle;
};

struct usb_request_callback {
	void (*to_dev)(uint16_t index, uint16_t value, const uint8_t *buf,
		       size_t size);
	int (*to_host)(uint16_t index, uint16_t value, uint8_t *buf,
		       size_t size);
};

#define USB_REQUEST_CALLBACK_DEFINE(_to_dev, _to_host)             \
	static const STRUCT_SECTION_ITERABLE(                      \
		usb_request_callback,                              \
		_usb_request_callback__##_to_dev##_##_to_host) = { \
		.to_dev = _to_dev,                                 \
		.to_host = _to_host,                               \
	}
