/* Copyright 2025 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "usb_request.h"

#include <zephyr/kernel.h>
#include <zephyr/platform/hooks.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>

#include <cmsis_core.h>
#include <soc.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_pwr.h>

/* Magic value stored in RTC->BKP0R across a cold reset to request DFU entry */
#define DFU_BOOT_MAGIC 0xDF59B007U

/* STM32H743 System Memory ROM bootloader addresses (RM0433 / AN2606) */
#define STM32H7_SYSMEM_BASE 0x1FF09800U
#define ROM_SYSTEM_INIT_ADDR 0x1FF123F5U
#define ROM_SCATTER_START_ADDR 0x1FF15DD4U
#define ROM_SCATTER_END_ADDR 0x1FF15E04U
#define ROM_CLOCK_MPU_INIT_ADDR 0x1FF0ADC5U
#define ROM_USB_DFU_INIT_ADDR 0x1FF09CC1U
#define ROM_USB_DFU_LOOP_ADDR 0x1FF09D01U
#define AXI_INI6_FN_MOD_ADDR 0x51008108U

#define AXI_SRAM_START 0x24000000U
#define AXI_SRAM_ROM_END 0x24005000U

typedef void (*rom_void_fn_t)(void);
typedef void (*rom_scatter_fn_t)(const void *src, void *dst, uint32_t len);

struct rom_scatter_entry {
	const void *src;
	void *dst;
	uint32_t len;
	uint32_t fn;
};

/*
 * Initialize the STM32H743 System Memory ROM bootloader C runtime and invoke
 * its clock, MPU, and USB OTG DFU routines directly.
 *
 * On the Dagwood board, PB15 (EN_PPVAR_MECC_RTC) has an external pull-down
 * resistor. The generic ROM peripheral scan loop (0x1FF0A398) checks PB15
 * (alternate USART1_RX) before USB enumeration completes; seeing PB15 low, it
 * assumes a UART start bit on USART1 and gets trapped in an infinite auto-baud
 * error loop (0x1FF0A594), preventing the main thread from ever reaching the
 * USB DFU loop (0x1FF09D00) that processes the DfuSe ":leave" reset.
 */
static __attribute__((noinline, noreturn)) void enter_rom_usb_dfu(void)
{
	const struct rom_scatter_entry *entry =
		(const struct rom_scatter_entry *)ROM_SCATTER_START_ADDR;
	const struct rom_scatter_entry *end =
		(const struct rom_scatter_entry *)ROM_SCATTER_END_ADDR;

	PWR->CR1 |= PWR_CR1_DBP;
	__DSB();
	RTC->BKP0R = 0U;
	__DSB();

	/*
	 * Allow ~30-50 ms at 64 MHz HSI with USB D+ pull-up disabled
	 * so the host detects USB disconnect before DFU enumeration.
	 */
	for (uint32_t i = 0U; i < 1000000U; i++) {
		__NOP();
	}

	/*
	 * Initialize 64-bit ECC words in the AXI SRAM region used by the ROM
	 * bootloader (.data, .bss, and stack at 0x240044B0).
	 */
	for (volatile uint64_t *p = (volatile uint64_t *)AXI_SRAM_START;
	     p < (volatile uint64_t *)AXI_SRAM_ROM_END; p++) {
		*p = 0ULL;
	}

	SCB->VTOR = STM32H7_SYSMEM_BASE;
	__DSB();
	__ISB();

	((rom_void_fn_t)ROM_SYSTEM_INIT_ADDR)();

	for (; entry < end; entry++) {
		rom_scatter_fn_t fn = (rom_scatter_fn_t)(entry->fn | 1U);

		fn(entry->src, entry->dst, entry->len);
	}

	__set_MSP(*(volatile uint32_t *)STM32H7_SYSMEM_BASE);

	*(volatile uint32_t *)AXI_INI6_FN_MOD_ADDR = 1U;
	((rom_void_fn_t)ROM_CLOCK_MPU_INIT_ADDR)();
	((rom_void_fn_t)ROM_USB_DFU_INIT_ADDR)();
	((rom_void_fn_t)ROM_USB_DFU_LOOP_ADDR)();
	CODE_UNREACHABLE;
}

void soc_early_reset_hook(void)
{
	/*
	 * When exiting the STM32H7 System Memory DFU bootloader via DfuSe
	 * ":leave", the ROM bootloader jumps directly to the application's
	 * Reset_Handler instead of issuing a system reset. This leaves
	 * Cortex-M7 L1 caches, PLLs, and the USB OTG peripheral active.
	 * Detect this warm jump and trigger a clean NVIC_SystemReset().
	 */
	if ((SCB->CCR & (SCB_CCR_IC_Msk | SCB_CCR_DC_Msk)) != 0U ||
	    (RCC->CR & (RCC_CR_HSEON | RCC_CR_PLL1ON | RCC_CR_PLL3ON)) != 0U ||
	    (RCC->AHB1ENR & RCC_AHB1ENR_USB2OTGHSEN) != 0U) {
		NVIC_SystemReset();
	}

	/*
	 * Check if the running firmware requested entry into the STM32 System
	 * Memory ROM DFU bootloader prior to triggering NVIC_SystemReset().
	 */
	RCC->APB4ENR |= RCC_APB4ENR_RTCAPBEN;
	__DSB();
	if (RTC->BKP0R == DFU_BOOT_MAGIC) {
		enter_rom_usb_dfu();
	}
}

static void enter_dfu_mode(void)
{
	LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_RTCAPB);
	LL_PWR_EnableBkUpAccess();
	RTC->BKP0R = DFU_BOOT_MAGIC;
	__DSB();

	sys_reboot(SYS_REBOOT_COLD);
}

static int cmd_dfu(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "Entering DFU bootloader...");
	k_msleep(50);
	enter_dfu_mode();

	return 0;
}

SHELL_CMD_REGISTER(dfu, NULL, "Reboot into STM32 DFU bootloader", cmd_dfu);

static void dw_reboot(uint16_t index, uint16_t value, const uint8_t *buf,
		      size_t size)
{
	if (index == USB_REQ_DFU || (index == USB_REQ_REBOOT && value != 0)) {
		enter_dfu_mode();
	} else if (index == USB_REQ_REBOOT) {
		sys_reboot(SYS_REBOOT_COLD);
	}
}

USB_REQUEST_CALLBACK_DEFINE(dw_reboot, NULL);
