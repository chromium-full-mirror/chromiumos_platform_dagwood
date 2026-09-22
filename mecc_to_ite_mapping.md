# MECC to ITE IT82002 Signal Mapping

This document provides the mapping between the 200-pin **MECC** (Modular
Embedded Controller Card) interface and the GPIO/peripheral pin assignments for
the **ITE IT82002** (IT8xxx2) EC Add-In Card (**`ite-aic`**).

## Overview

- **EC Architecture**: ITE Tech IT82002 (IT82002VG / IT82002AW)
- **Reference AIC Board**: `ite-aic` (`zephyr/program/ec-aic/ite-aic`)
- **Reference RVP Board**: `ocelotrvp-ite` (`zephyr/program/ocelot/ocelotrvp-ite`) / `fatcatrvp-ite`
- **Supported Standards**: Intel MECC Specification Revision 2.1

---

## Signal Mapping Grouped by ITE Port

| MECC Pin | MECC Signal Name | ITE Signal / GPIO | Direction / Level | Devicetree Node / Net Name | Function / Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Port A Signals** | | | | | |
| 181 | VOL_DN_EC | GPA0 | In (3.3V) | `gpio_ec_voldn_btn_odl` | Volume Down button |
| 83 | KBD_BKLT_CTRL | GPA1 | Out / PWM (3.3V) | `pwm1` | Keyboard backlight PWM control |
| - | LED_1 | GPA2 | Out (3.3V) | `led_1` | Status LED 1 |
| - | LED_2 | GPA3 | Out (3.3V) | `led_2` | Status LED 2 |
| 174 | VOLUME_UP | GPA4 | In (3.3V) | `gpio_ec_volup_btn_odl` | Volume Up button |
| 36 | MECC_CPU_FAN_PWM | GPA5 | Out / PWM (3.3V) | `pwm5` | CPU Fan PWM drive control |
| 138 | STD_ADP_PRSNT | GPA6 | In (3.3V) | `gpio_std_adp_prsnt_r` | DC barrel adapter present detect |
| 26 | MECC_STD_ADPT_CNTRL | GPA7 | Out (3.3V) | `gpio_std_adpt_cntrl_gpio_r` | Standard adapter power control |
| **Port B Signals** | | | | | |
| 178 | EC_UART_RX | GPB0 | In / UART (3.3V) | `uart0 (SIN0)` | EC UART console RX (from Dagwood/AP) |
| 185 | EC_UART_TX | GPB1 | Out / UART (3.3V) | `uart0 (SOUT0)` | EC UART console TX (to Dagwood/AP) |
| 119 | CPU_C10_GATE | GPB2 | In (1.8V) | `gpio_cpu_c10_gate_r` | CPU C10 gate power status |
| 79 / 156 | SMCLK0 / SMB_PCH_CK | GPB3 | In/Out / I2C (3.3V/1.8V) | `i2c0_clk` | I2C0 clock (Target mode / PCH I2C) |
| 81 / 161 | SMDAT0 / SMB_PCH_DATA | GPB4 | In/Out / I2C (3.3V/1.8V) | `i2c0_data` | I2C0 data (Target mode / PCH I2C) |
| 192 | MECC_WP_ODL | GPB5 | In (3.3V) | `gpio_c_ec_wp_odl_r` | Flash Write Protect input (active-low) |
| 99 | MECC_EC_PCH_WAKE_ODL | GPB6 | Out / OD (3.3V) | `gpio_smc_wake_sci_n_r` | EC to PCH Wake SCI (SMC_WAKE_SCI_N) |
| - | - | GPB7 | - | - | Reserved |
| **Port C Signals** | | | | | |
| 108 | MECC_ACOK | GPC0 | In (3.3V) | `gpio_acok_od` | Charger AC OK present detect |
| 78 | WC_USBC_PD_CLK_AIC1 | GPC1 | Out / OD (3.3V) | `i2c1_clk` | I2C1 clock (USB Type-C PD Port 1) |
| 86 | WC_USBC_PD_DATA_AIC1 | GPC2 | In/Out / OD (3.3V) | `i2c1_data` | I2C1 data (USB Type-C PD Port 1) |
| - | - | GPC3 | - | - | Reserved |
| 66 | MECC_EN_PP5000_FAN | GPC4 | Out (3.3V) | `gpio_ec_fan_pwr_disable_n_r` | 5V Fan power gate control |
| - | - | GPC5 | - | - | Reserved |
| 100 | MECC_EN_EDP_BKLT | GPC6 | Out (3.3V) | `gpio_c_edp_bklt_en_r` | eDP Backlight enable |
| 10 | C_USBC_PD_CLK_AIC3_4 | GPC7 | Out / OD (3.3V) | `i2c2_clk` | I2C2 clock (USB Type-C PD Port 3/4) |
| **Port D Signals** | | | | | |
| 27 | C_USBC_PD_DATA_AIC3_4 | GPD0 | In/Out / OD (3.3V) | `i2c2_data` | I2C2 data (USB Type-C PD Port 3/4) |
| 74 | MECC_TABLET_MODE_L | GPD1 | In (1.8V) | `gpio_tablet_mode_l` | Tablet / Clamshell mode hall sensor |
| 63 / 116 | MECC_EC_RST_L / PM_SLP_A_N | GPD2 | In (1.8V/3.3V) | `pwr_slp_a / gpio_ec_rst_l` | EC Reset / PCH SLP_A# |
| 123 | MECC_LID_OPEN | GPD3 | In (3.3V) | `gpio_lid_open` | Lid open / closed sensor (SMC_LID) |
| 87 | MECC_SLP_SUS_L | GPD4 | In (1.8V) | `pwr_slp_sus` | PCH SLP_SUS# signal |
| 22 | MECC_ALL_SYS_PWRGD | GPD5 | In (3.3V) | `gpio_all_sys_pwrgd_r` | All system power rails good indicator |
| 42 | MECC_CPU_FAN_TACH | GPD6 | In / Tach (3.3V) | `tach0 (TACH0A)` | CPU Fan tachometer feedback |
| 104 | VIRTUAL_BATTERY | GPD7 | In (3.3V) | `gpio_virtual_battery_r` | Virtual battery presence switch |
| **Port E Signals** | | | | | |
| 160 | SMB_BS_CLK | GPE0 | Out / OD (3.3V) | `i2c4_clk` | I2C4 clock (Battery / Charger / CBI) |
| 73 | C_ISH_EC_CLK | GPE1 | In/Out / OD (1.8V) | `i2c5_clk` | I2C5 clock (EC to ISH / Sensors) |
| 127 | C_ISH_EC_DATA | GPE2 | In/Out / OD (1.8V) | `i2c5_data` | I2C5 data (EC to ISH / Sensors) |
| 57 | MECC_SOC_PWRBTN_L | GPE3 | Out / OD (1.8V) | `gpio_pm_pwrbtn_n_r` | PCH Power Button (PM_PWRBTN_N) |
| 59 / 115 | MECC_GSC_PWRBTN_L | GPE4 | In (3.3V) | `gpio_gsc_ec_pwr_btn_odl` | Power button input from GSC/Header |
| 126 | MECC_EC_BATT_PRES_ODL | GPE5 | In (3.3V) | `gpio_bat_det_r` | Battery detect input (BAT_DET) |
| 135 | MECC_CCD_MODE_L | GPE6 | In (3.3V) | `gpio_ccd_mode_odl` | Case Closed Debugging (CCD) mode status |
| 165 | SMB_BS_DATA | GPE7 | In/Out / OD (3.3V) | `i2c4_data` | I2C4 data (Battery / Charger / CBI) |
| **Port F Signals** | | | | | |
| 7 | MECC_RSMRST_PWRGD_L | GPF0 | In (1.8V) | `pwr_rsmrst_pwrgd` | Resume Reset power good (RSMRST_PWRGD_N) |
| 105 | MECC_SYS_PWROK | GPF1 | Out / OD (1.8V) | `pwr_sys_pwrok` | System Power OK (SYS_PWROK_EC) |
| 156 | SMB_PCH_CK (alt) | GPF2 | In/Out / OD (1.8V) | `gpf2` | Alt PCH SMBUS clock / Trackpad |
| 161 | SMB_PCH_DATA (alt) | GPF3 | In/Out / OD (1.8V) | `gpf3` | Alt PCH SMBUS data / Trackpad |
| 145 | MECC_EC_PP5000_USBA | GPF4 | Out (3.3V) | `gpio_en_pp5000_usba` | USB Type-A 5V power rail enable |
| 6 | I2C_MECC_PDC1_INT_L | GPF5 | In (3.3V) | `gpio_wc_usbc_pd_alrt_aic1_r` | Type-C PD Port 1 alert interrupt |
| 29 / 85 | I2C_MECC_PDC4_INT_L | GPF6 | In (1.8V/3.3V) | `gpio_c_ec_snr_alrt2` | Type-C PD Port 4 alert / Sensor alert 2 |
| 65 | I2C_MECC_PDC3_INT_L | GPF7 | In (1.8V/3.3V) | `gpio_c_ec_snr_alrt1` | Type-C PD Port 3 alert / Sensor alert 1 |
| **Port G Signals** | | | | | |
| 142 | ESPI_MECC_ALERT0_L | GPG0 | Out / OD (1.8V) | `espi_alert0 (SSCE1#)` | eSPI alert 0 |
| 91 | SPI_MECC_CS1_L | GPG1 | Out (1.8V) | `ec_spi_cs1_n` | SPI Flash Chip Select 1 |
| 111 | ESPI_MECC_RESET_L | GPG2 | In (1.8V) | `espi_rst_n (SSCE0#)` | eSPI Reset (active-low) |
| 120 / 149 | SPI_MECC_CS0_L / OE_L | GPG3 | Out (1.8V) | `fsce_l_r` | SPI Flash CS0# / Output Enable |
| 143 | SPI_MECC_SOC_DO_AIC_DI | GPG4 | In / SPI (1.8V) | `fmosi_r` | SPI Flash MOSI |
| 136 | SPI_MECC_SOC_DI_AIC_DO | GPG5 | Out / SPI (1.8V) | `fmiso_r` | SPI Flash MISO |
| 82 | ESPI_MECC_ALERT2_L | GPG6 | In (1.8V) | `gpg6_r (DSR0#)` | eSPI alert 2 |
| 128 | SPI_MECC_CLK | GPG7 | Out / SPI (1.8V) | `fsck_r` | SPI Flash Clock |
| **Port H Signals** | | | | | |
| 28 | MECC_AMP_MUTE_ODL | GPH0 | Out / OD (1.8V) | `gpio_c_ec_pdb_ctrl_r` | Audio Codec Amp Mute (PDB_CTRL) |
| 132 | WC_USBC_PD_CLK_AIC2 | GPH1 | Out / OD (3.3V) | `i2c3_clk` | I2C3 clock (USB Type-C PD Port 2) |
| 124 | WC_USBC_PD_DATA_AIC2 | GPH2 | In/Out / OD (3.3V) | `i2c3_data` | I2C3 data (USB Type-C PD Port 2) |
| 49 | MECC_SLP_S3_L | GPH3 | In (1.8V) | `pwr_slp_s3` | PCH SLP_S3# sleep state |
| 64 | MECC_SLP_S4_L | GPH4 | In (1.8V) | `pwr_slp_s4` | PCH SLP_S4# sleep state |
| 159 | IO_EXP_INT_MECC | GPH5 | In (3.3V) | `gpio_io_exp_int_mecc_r` | Baseboard IO expander interrupt |
| 151 | I2C_MECC_ISH_INT_L | GPH6 | Out / OD (1.8V) | `gpio_ec_ish_int_odl` | EC to ISH interrupt / alert |
| **Port I Signals** | | | | | |
| 169 | MECC_PROCHOT_L | GPI0 | In/Out / OD (3.3V) | `gpio_prochot_odl` | Processor Hot (PROCHOT#) |
| 12 | ADC_MECC_AMON | GPI1 | In / ADC (3.3V) | `adc_charger_amon_bmon` | Charger current monitor (AMON/BMON) |
| 33 | ADC_MECC_VR_TEMP | GPI2 | In / ADC (3.3V) | `adc_temp_vr` | Voltage regulator thermistor |
| 56 | ADC_MECC_SKIN_TEMP | GPI3 | In / ADC (3.3V) | `adc_temp_skin` | Skin temperature thermistor |
| 69 | ADC_MECC_DDR_TEMP | GPI4 | In / ADC (3.3V) | `adc_temp_ddr_soc` | DDR / SoC memory thermistor |
| 47 | ADC_MECC_AMB_TEMP | GPI5 | In / ADC (3.3V) | `adc_temp_ambient` | Ambient temperature thermistor |
| 172 | C_IOEX_KBD_INTR | GPI6 | In (3.3V) | `gpio_c_ioex_kbd_intr_r` | Discrete keyboard IO expander interrupt |
| 55 | MECC_EN_S5_RAILS | GPI7 | Out (3.3V) | `pwr_en_s5_rails` | S5 rails enable (EC_DS3) |
| **Port J Signals** | | | | | |
| 48 / 191 | BUF_PLT_RST_N / RSMRST_L | GPJ0 | In/Out (1.8V) | `pwr_plt_rst / pwr_ec_pch_rsmrst` | Platform Reset in / RSMRST# out |
| 30 | MECC_PLT_PWROK | GPJ1 | Out (1.8V) | `pwr_pch_pwrok` | Platform / PCH Power OK (PCH_PWROK_EC) |
| 38 | MECC_BOOT_STALL | GPJ2 | In (3.3V) | `boot_stall` | SoC boot stall indication |
| 61 / 164 | EN_SLP_Z / CSE_EARLY_SW | GPJ3 | Out / OD (1.8V/3.3V) | `gpio_c_mecc_cse_early_sw_r` | CSE Early Recovery switch / EN_SLP_Z |
| 187 | MECC_EC_PCH_INT_ODL | GPJ4 | Out / OD (1.8V) | `gpio_ec_soc_int_odl` | EC to PCH host interrupt (MKBP sync) |
| 35 / 167 | SLP_S5_L / SLP_S0_L | GPJ5 | In (1.8V) | `pwr_slp_s5 / pwr_slp_s0` | PCH SLP_S5# / SLP_S0# sleep states |
| 118 | I2C_MECC_PDC2_INT_L | GPJ6 | In (3.3V) | `gpio_wc_usbc_pd_alrt_aic2_r` | Type-C PD Port 2 alert interrupt |
| 134 | MECC_SYS_RST_ODL | GPJ7 | Out / OD (3.3V) | `gpio_rst_push_n_d_r` | SoC / Platform System Reset |

---

## Complete MECC Pin to ITE GPIO Cross-Reference

| MECC Pin | MECC Signal Name | ITE Pin / GPIO | Direction | Voltage Rail | Function / Role |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **2** | GND | GND | Power | GND | Ground |
| **3** | V3.3A | V3.3A | Power | PP3300_MECC_Z | 3.3V Always-on rail |
| **4** | V3.3A | V3.3A | Power | PP3300_MECC_Z | 3.3V Always-on rail |
| **5** | GND | GND | Power | GND | Ground |
| **6** | WC_USBC_PD_ALRT_AIC1 | GPF5 | Input | V3.3_EC_Core | Type-C PD Port 1 alert |
| **7** | RSMRST_PWRGD_N | GPF0 (SIN1) | Input | +V1P8A | Resume Reset Power Good |
| **8** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **9** | GND | GND | Power | GND | Ground |
| **10** | C_USBC_PD_CLK_AIC3_4 | GPC7 (I2C2 SCL) | Bidirectional | V3.3_EC_Core | I2C2 Clock (PD Port 3/4) |
| **11** | GND | GND | Power | GND | Ground |
| **12** | C_CHRG_AMON/BMON | GPI1 (ADC1) | Input / ADC | V3.3_EC_Core | Charger current monitor |
| **13** | C_KSO_02 | GPH10 (KSO2) | Output | V3.3_EC_Core | Keyboard scan out 2 |
| **14** | GND | GND | Power | GND | Ground |
| **15** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **16** | GND | GND | Power | GND | Ground |
| **17** | CAPS_LOCK | - | Output | V3.3_EC_Core | Keyboard Caps Lock LED |
| **18** | NUM_LOCK | - | Output | V3.3_EC_Core | Keyboard Num Lock LED |
| **19** | SCROLL_LOCK | - | Output | V3.3_EC_Core | Keyboard Scroll Lock LED |
| **20** | GND | GND | Power | GND | Ground |
| **21** | C_KSI_00 | GPH13 (KSI0) | Input | V3.3_EC_Core | Keyboard scan in 0 |
| **22** | ALL_SYS_PWRGD | GPD5 | Input | V3.3S | All System Power Good |
| **23** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **24** | GND | GND | Power | GND | Ground |
| **25** | C_EC_KSI_00 | GPH14 (EC_KSI0)| Input | V3.3_EC_Core | Keyboard scan in 0 to EC |
| **26** | STD_ADPT_CNTRL_GPIO | GPA7 | Output | V3.3_EC_Core | Standard adapter power gate |
| **27** | C_USBC_PD_DATA_AIC3_4| GPD0 (I2C2 SDA) | Bidirectional | V3.3_EC_Core | I2C2 Data (PD Port 3/4) |
| **28** | C_EC_PDB_CTRL | GPH0 (CLKRUN#)| Output / OD | +V1P8A | Audio Amp Mute control |
| **29** | C_USBC_PD_ALRT_AIC4 | GPF6 | Input | V3.3_EC_Core | Type-C PD Port 4 alert |
| **30** | PCH_PWROK_EC | GPJ1 | Output / OD | +V1P8S | Platform Power OK |
| **32** | GND | GND | Power | GND | Ground |
| **33** | VR_TEMP/V_1P05_PCH_EC | GPI2 (ADC2) | Input / ADC | V3.3_EC_Core | VR temperature sensor |
| **35** | PM_SLP_S5_N | GPJ5 (RIG0#) | Input | +V1P8A | PCH SLP_S5# sleep status |
| **36** | CPU_PWM_FAN | GPA5 (PWM5) | Output / PWM | V3.3_EC_Core | Fan PWM drive control |
| **38** | Boot_Stall | GPJ2 | Input | V3.3_EC_Core | SoC boot stall status |
| **39** | GND | GND | Power | GND | Ground |
| **40** | C_EC_KSO_02_INV | GPI9 (KSO2_INV)| Input | V3.3_EC_Core | Inverted KSO2 from GSC |
| **41** | PM_BATLOW_N | GPB2 | Input | +V1P8A | PCH Battery Low status |
| **42** | CPU_TACHO_FAN | GPD6 (TACH0A) | Input / Tach | V3.3_EC_Core | Fan tachometer feedback |
| **45** | C_KSI_02_03 | GPH12 (KSI2) | Input | V3.3_EC_Core | Keyboard scan in 2/3 |
| **46** | C_EC_KSI_02_03 | GPH15 (EC_KSI2)| Input | V3.3_EC_Core | Keyboard scan in 2/3 to EC |
| **47** | AMBIENT_THERM_IN0 | GPI5 (ADC5) | Input / ADC | V3.3_EC_Core | Ambient thermistor |
| **48** | BUF_PLT_RST_N | GPJ0 (TACH2) | Input | +V1P8A | Platform Reset from SoC |
| **49** | PM_SLP_S3_N | GPH3 | Input | +V1P8A | PCH SLP_S3# sleep status |
| **52** | GND | GND | Power | GND | Ground |
| **55** | EC_DS3 | GPI7 (CTS1#) | Output | V3.3_EC_Core | S5 rails enable (EN_S5_RAILS) |
| **56** | SKIN_THERM_IN | GPI3 (ADC3) | Input / ADC | V3.3_EC_Core | Skin temperature thermistor |
| **57** | PM_PWRBTN_N | GPE3 | Output / OD | +V1P8A | PCH Power Button output |
| **58** | GND | GND | Power | GND | Ground |
| **59** | C_GSC_EC_PWR_BTN_N | GPE4 (PWRSW) | Input | V3.3_EC_Core | Power button input from GSC |
| **60** | +V5A | +V5A | Power | PP5000_MECC_Z | 5V System Power rail |
| **61** | EC_PG3_WAKE | GPJ3 | Output | V3.3A_RTC | Latch enable (EN_SLP_Z) |
| **63** | SMC_RST | GPD2 (WRST#) | Input | V3.3A_RTC | EC Reset from Dagwood |
| **64** | PM_SLP_S4_N | GPH4 | Input | +V1P8A | PCH SLP_S4# sleep status |
| **65** | C_USBC_PD_ALRT_AIC3 | GPF7 | Input | V3.3_EC_Core | Type-C PD Port 3 alert |
| **66** | EC_FAN_PWR_DISABLE_N | GPC4 | Output | V3.3_EC_Core | Fan 5V power gate control |
| **67** | GND | GND | Power | GND | Ground |
| **69** | DDR_THERM_IN | GPI4 (ADC4) | Input / ADC | V3.3_EC_Core | DDR / SoC memory thermistor |
| **70** | GND | GND | Power | GND | Ground |
| **73** | C_ISH_EC_CLK | GPE1 | Bidirectional | +V1P8A | I2C5 Clock (EC-ISH interface) |
| **74** | EC_SLATEMODE_HALLOUT_SNSR | GPD1 | Input | +V1P8A | Tablet mode hall sensor |
| **77** | GND | GND | Power | GND | Ground |
| **78** | WC_USBC_PD_CLK_AIC1 | GPC1 (I2C1 SCL) | Bidirectional | V3.3_EC_Core | I2C1 Clock (PD Port 1) |
| **79** | SMCLK0 | GPB3 (I2C0 SCL) | Bidirectional | V3.3_EC_Core | Primary Dagwood Target I2C SCL |
| **80** | EC_1.8V_A_rail | +V1P8A | Power | PP1800_MECC_S | 1.8V Switched Power rail |
| **81** | SMDAT0 | GPB4 (I2C0 SDA) | Bidirectional | V3.3_EC_Core | Primary Dagwood Target I2C SDA |
| **82** | ESPI_ALERT2_AIC_N | GPG6 (DSR0#) | Input | +V1P8A | eSPI Alert 2 |
| **83** | KBD_BKLT_CTRL | GPA1 (PWM1) | Output / PWM | V3.3_EC_Core | Keyboard backlight PWM |
| **84** | GND | GND | Power | GND | Ground |
| **85** | ESPI_ALERT1_AIC_N | GPF6 | Input | +V1P8A | eSPI Alert 1 |
| **86** | WC_USBC_PD_DATA_AIC1 | GPC2 (I2C1 SDA) | Bidirectional | V3.3_EC_Core | I2C1 Data (PD Port 1) |
| **87** | D_PM_SLP_SUS_EC_N | GPD4 | Input | +V1P8A | PCH SLP_SUS# sleep status |
| **88** | EC_1.8V_A_rail | +V1P8A | Power | PP1800_MECC_S | 1.8V Switched Power rail |
| **91** | EC_SPI_CS1_N | GPG1 | Output | +V1P8A | SPI Flash Chip Select 1 |
| **93** | PM_SLP_LAN_N | GPE5 | Input | +V1P8A | PCH SLP_LAN# sleep status |
| **94** | GND | GND | Power | GND | Ground |
| **95** | PCH_SLP_WLAN_R_N | GPE6 | Input | +V1P8A | PCH SLP_WLAN# sleep status |
| **96** | GND | GND | Power | GND | Ground |
| **97** | CPU_CAT_ERR | GPC0 | Input | V3.3_EC_Core | CPU Catastrophic Error |
| **99** | SMC_WAKE_SCI_N | GPB6 | Output / OD | V3.3A | EC to PCH Wake SCI |
| **100** | C_EDP_BKLT_EN | GPC6 | Output | V3.3_EC_Core | eDP Backlight enable |
| **104** | VIRTUAL_BATTERY | GPD7 | Input | V3.3A_RTC | Virtual battery present switch |
| **105** | SYS_PWROK_EC | GPF1 (SOUT1) | Output / OD | +V1P8S | System Power OK |
| **108** | BC_ACOK | GPC0 | Input | V3.3A_RTC | Charger AC OK present detect |
| **111** | ESPI_RST_N | GPG2 (SSCE0#) | Input | +V1P8A | eSPI Reset (active-low) |
| **112** | GND | GND | Power | GND | Ground |
| **115** | PWRBTN_EC_IN_N | GPE4 (PWRSW) | Input | V3.3A_RTC | Power button input |
| **116** | PM_SLP_A_N | GPD2 | Input | +V1P8A | PCH SLP_A# sleep status |
| **118** | WC_USBC_PD_ALRT_AIC2 | GPJ6 | Input | V3.3_EC_Core | Type-C PD Port 2 alert |
| **119** | CPU_C10_GATE | GPB2 | Input | +V1P8A | CPU C10 gate power status |
| **120** | EC_SPI_OE_N | GPG3 | Output | +V1P8A | Shared SPI Flash Output Enable |
| **123** | SMC_LID | GPD3 | Input | V3.3A_RTC | Lid open/closed sensor switch |
| **124** | WC_USBC_PD_DATA_AIC2 | GPH2 (I2C3 SDA) | Bidirectional | V3.3_EC_Core | I2C3 Data (PD Port 2) |
| **126** | BAT_DET | GPE5 | Input | V3.3_EC_Core | Battery pack detect |
| **127** | C_ISH_EC_DATA | GPE2 | Bidirectional | +V1P8A | I2C5 Data (EC-ISH interface) |
| **128** | EC_SPI_CK | GPG7 | Output / SPI | +V1P8A | SPI Flash Clock |
| **132** | WC_USBC_PD_CLK_AIC2 | GPH1 (I2C3 SCL) | Bidirectional | V3.3_EC_Core | I2C3 Clock (PD Port 2) |
| **134** | RST_PUSH_N_D | GPJ7 | Output / OD | V3.3_EC_Core | System Reset (SYS_RST_ODL) |
| **135** | C_CCD_MODE_ODL | GPE6 | Input | V3.3_EC_Core | CCD Mode detect |
| **136** | EC_SPI_SO (MISO) | GPG5 | Input / SPI | +V1P8A | SPI Flash MISO |
| **138** | STD_ADP_PRSNT | GPA6 | Input | V3.3_EC_Core | DC barrel adapter detect |
| **142** | eSPI_ALERT0_N | GPG0 (SSCE1#) | Output / OD | +V1P8A | eSPI alert 0 |
| **143** | EC_SPI_SI (MOSI) | GPG4 | Output / SPI | +V1P8A | SPI Flash MOSI |
| **145** | EN_PP5000_USBA | GPF4 | Output | V3.3_EC_Core | USB Type-A 5V power enable |
| **149** | EC_SPI_CS0_N | GPG3 | Output | +V1P8A | SPI Flash Chip Select 0 |
| **151** | C_EC_ISH_ALRT | GPH6 | Output / OD | +V1P8A | EC to ISH interrupt |
| **156** | SMB_PCH_CK | GPB3 / GPF2 (I2C0 SCL) | Bidirectional | +V1P8A | PCH SMBUS clock |
| **159** | IO_EXP_INT_MECC | GPH5 | Input | V3.3_EC_Core | Baseboard IO expander interrupt |
| **160** | SMB_BS_CLK | GPE0 (I2C4 SCL) | Bidirectional | V3.3_EC_Core | I2C4 Clock (Battery / Charger) |
| **161** | SMB_PCH_DATA | GPB4 / GPF3 (I2C0 SDA) | Bidirectional | +V1P8A | PCH SMBUS data |
| **163** | eSPI_CS0_N | SSCE0# (eSPI CS0#) | Input | +V1P8A | eSPI Chip Select 0 |
| **164** | C_MECC_CSE_EARLY_SW | GPJ3 | Output / OD | +V1P8A | CSE Early Recovery switch |
| **165** | SMB_BS_DATA | GPE7 (I2C4 SDA) | Bidirectional | V3.3_EC_Core | I2C4 Data (Battery / Charger) |
| **167** | PCH_SLP_S0_N | GPJ5 (RIG0#) | Input | +V1P8A | PCH SLP_S0# sleep status |
| **169** | PROCHOT_EC_N | GPI0 | Output / OD | V3.3_EC_Core | Processor Hot alert (PROCHOT#) |
| **172** | C_IOEX_KBD_INTR | GPI6 | Input | V3.3_EC_Core | Keyboard IO expander interrupt |
| **174** | VOLUME_UP | GPA4 | Input | V3.3_EC_Core | Volume Up button |
| **177** | ESPI_IO[2] | SIO2 (eSPI IO2) | Bidirectional | +V1P8A | eSPI Data IO2 |
| **178** | EC_UART_RX | GPB0 (SIN0) | Input / UART | V3.3_EC_Core | EC Console UART RX |
| **181** | VOL_DN_EC | GPA0 | Input | V3.3_EC_Core | Volume Down button |
| **184** | ESPI_IO[1] | SIO1 (eSPI IO1) | Bidirectional | +V1P8A | eSPI Data IO1 |
| **185** | EC_UART_TX | GPB1 (SOUT0)| Output / UART| V3.3_EC_Core | EC Console UART TX |
| **187** | SMC_EXTSMI_N / PCH_INT| GPJ4 | Output / OD | +V1P8A | EC to PCH host interrupt (MKBP)|
| **190** | ESPI_IO[0] | SIO0 (eSPI IO0) | Bidirectional | +V1P8A | eSPI Data IO0 |
| **191** | PM_RSMRST_N | GPJ0 | Output | +V1P8A | Resume Reset output to SoC |
| **192** | C_EC_WP_ODL | GPB5 | Input | V3.3_EC_Core | Flash Write Protect input |
| **198** | ESPI_IO[3] | SIO3 (eSPI IO3) | Bidirectional | +V1P8A | eSPI Data IO3 |
| **199** | ESPI_CLK | SCK (eSPI CLK) | Input / eSPI | +V1P8A | eSPI Bus Clock |
