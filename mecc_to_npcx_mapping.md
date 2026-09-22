# MECC to NPCX NPCX9 Signal Mapping

This document provides the mapping between the 200-pin **MECC** (Modular
Embedded Controller Card) interface and the GPIO/peripheral pin assignments for
the **Nuvoton NPCX9** (NPCX9m7f / NPCX997F) EC Add-In Card (**`npcx-aic`**).

## Overview

- **EC Architecture**: Nuvoton NPCX9 Series (NPCX9m7f / NPCX997FA0BX)
- **Reference AIC Board**: `npcx-aic` (`zephyr/program/ec-aic/npcx-aic`)
- **Reference RVP Board**: `ocelotrvp-npcx` (`zephyr/program/ocelot/ocelotrvp-npcx`) / `fatcatrvp-npcx`
- **Supported Standards**: Intel MECC Specification Revision 2.1

---

## Signal Mapping Grouped by NPCX Port

| MECC Pin | MECC Signal Name | NPCX GPIO | Direction / Level | Devicetree Node / Net Name | Function / Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Port 0 Signals** | | | | | |
| 115 / 59 | PWRBTN_EC_IN_N | GPIO00 | In (3.3V) | `gpio_mech_pwr_btn_odl` | Power button input from button / GSC |
| 123 | MECC_LID_OPEN | GPIO01 | In (3.3V) | `gpio_lid_open` | Lid open / closed sensor switch (SMC_LID) |
| 118 | I2C_MECC_PDC2_INT_L | GPIO02 | In (3.3V) | `usb_pd2_int_odl` | Type-C PD Port 2 alert interrupt |
| - | KSO15 | GPIO03 | Out (3.3V) | `aic-pins` | Keyboard scan out 15 |
| - | KSO12 | GPIO04 | Out (3.3V) | `aic-pins` | Keyboard scan out 12 |
| - | KSO11 | GPIO05 | Out (3.3V) | `aic-pins` | Keyboard scan out 11 |
| - | KSO10 | GPIO06 | Out (3.3V) | `aic-pins` | Keyboard scan out 10 |
| - | KSO09 | GPIO07 | Out (3.3V) | `aic-pins` | Keyboard scan out 9 |
| **Port 1 Signals** | | | | | |
| - | KSO08 | GPIO10 | Out (3.3V) | `aic-pins` | Keyboard scan out 8 |
| - | KSO07 | GPIO11 | Out (3.3V) | `aic-pins` | Keyboard scan out 7 |
| - | KSO06 | GPIO12 | Out (3.3V) | `aic-pins` | Keyboard scan out 6 |
| - | KSO05 | GPIO13 | Out (3.3V) | `aic-pins` | Keyboard scan out 5 |
| - | KSO04 | GPIO14 | Out (3.3V) | `aic-pins` | Keyboard scan out 4 |
| - | KSO03 | GPIO15 | Out (3.3V) | `aic-pins` | Keyboard scan out 3 |
| - | KSO02 | GPIO16 | Out (3.3V) | `aic-pins` | Keyboard scan out 2 |
| - | - | GPIO17 | - | - | Reserved |
| **Port 2 Signals** | | | | | |
| - | KSO01 | GPIO20 | Out (3.3V) | `aic-pins` | Keyboard scan out 1 |
| - | KSO00 | GPIO21 | Out (3.3V) | `aic-pins` | Keyboard scan out 0 |
| - | KSI07 | GPIO22 | In (3.3V) | `aic-pins` | Keyboard scan in 7 |
| - | KSI06 | GPIO23 | In (3.3V) | `aic-pins` | Keyboard scan in 6 |
| - | KSI05 | GPIO24 | In (3.3V) | `aic-pins` | Keyboard scan in 5 |
| - | KSI04 | GPIO25 | In (3.3V) | `aic-pins` | Keyboard scan in 4 |
| - | KSI03 | GPIO26 | In (3.3V) | `aic-pins` | Keyboard scan in 3 |
| - | KSI02 | GPIO27 | In (3.3V) | `aic-pins` | Keyboard scan in 2 |
| **Port 3 Signals** | | | | | |
| - | KSI01 | GPIO30 | In (3.3V) | `aic-pins` | Keyboard scan in 1 |
| - | KSI00 | GPIO31 | In (3.3V) | `aic-pins` | Keyboard scan in 0 |
| 99 | MECC_EC_PCH_WAKE_ODL | GPIO32 | Out / OD (1.8V) | `gpio_ec_soc_wake_ls_odl` | EC to PCH Wake SCI (SMC_WAKE_SCI_N) |
| 73 | C_ISH_EC_CLK | GPIO33 | In/Out / I2C (1.8V) | `i2c5_0_scl` | I2C5 clock (EC to ISH / Sensors) |
| - | EC_ACCEL_INT_L | GPIO34 | In (1.8V) | `gpio_ec_accel_int_l` | Base accelerometer interrupt |
| 169 | MECC_PROCHOT_L | GPIO35 | Out / OD (1.8V) | `gpio_ec_prochot_ls_odl` | Processor Hot (PROCHOT_EC_N) |
| 127 | C_ISH_EC_DATA | GPIO36 | In/Out / I2C (1.8V) | `i2c5_0_sda` | I2C5 data (EC to ISH / Sensors) |
| 12 | ADC_MECC_AMON | GPIO37 | In / ADC (3.3V) | `adc0_chan5` | Charger current monitor (AMON/BMON) |
| **Port 4 Signals** | | | | | |
| 42 | MECC_CPU_FAN_TACH | GPIO40 | In / Tach (3.3V) | `tach1 (TA1_1)` | CPU Fan tachometer input |
| 55 | MECC_EN_S5_RAILS | GPIO41 | Out (3.3V) | `pwr_en_pp3300_s5 / en_s5_rails` | S5 rails enable (EC_DS3) |
| 69 | ADC_MECC_DDR_TEMP | GPIO41 | In / ADC (3.3V) | `adc0_chan3` | DDR / SoC memory thermistor |
| 33 | ADC_MECC_VR_TEMP | GPIO43 | In / ADC (3.3V) | `adc0_chan10` | Voltage regulator thermistor |
| 135 | MECC_CCD_MODE_L | GPIO43 | In (3.3V) | `gpio_ccd_mode_odl` | CCD mode status detect |
| 56 | ADC_MECC_SKIN_TEMP | GPIO44 | In / ADC (3.3V) | `adc0_chan1` | Skin temperature thermistor |
| 47 | ADC_MECC_AMB_TEMP | GPIO45 | In / ADC (3.3V) | `adc0_chan0` | Ambient temperature thermistor |
| 163 | eSPI_CS0_N | GPIO46 | In / eSPI (1.8V) | `espi_lpc` | eSPI Chip Select 0 |
| 142 | ESPI_MECC_ALERT0_L | GPIO47 | Out / OD (1.8V) | `espi_lpc` | eSPI alert 0 |
| **Port 5 Signals** | | | | | |
| 174 | VOLUME_UP | GPIO50 | In (3.3V) | `gpio_ec_volup_btn_odl` | Volume Up button |
| 190 | ESPI_IO[0] | GPIO51 | In/Out / eSPI (1.8V) | `espi_lpc` | eSPI Data IO0 |
| 184 | ESPI_IO[1] | GPIO52 | In/Out / eSPI (1.8V) | `espi_lpc` | eSPI Data IO1 |
| 177 | ESPI_IO[2] | GPIO53 | In/Out / eSPI (1.8V) | `espi_lpc` | eSPI Data IO2 |
| 198 | ESPI_IO[3] | GPIO54 | In/Out / eSPI (1.8V) | `espi_lpc` | eSPI Data IO3 |
| 199 | ESPI_CLK | GPIO55 | In / eSPI (1.8V) | `espi_lpc` | eSPI Bus Clock |
| - | - | GPIO56 | - | - | Reserved |
| 111 | ESPI_MECC_RESET_L | GPIO57 | In / eSPI (1.8V) | `espi_lpc` | eSPI Reset (active-low) |
| **Port 6 Signals** | | | | | |
| 30 | MECC_PLT_PWROK | GPIO61 | Out (1.8V) | `pwr_pch_pwrok` | Platform / PCH Power OK (PCH_PWROK_EC) |
| 138 | STD_ADP_PRSNT | GPIO62 | In (3.3V) | `gpio_adp_det` | DC barrel adapter present detect |
| 22 | MECC_ALL_SYS_PWRGD | GPIO63 | In (3.3V) | `gpio_all_sys_pwrgd` | All system power rails good indicator |
| 178 | EC_UART_RX | GPIO64 | In / UART (3.3V) | `cr_sin1` | EC UART console RX (from Dagwood/AP) |
| 185 | EC_UART_TX | GPIO65 | Out / UART (3.3V) | `cr_sout1` | EC UART console TX (to Dagwood/AP) |
| 26 | MECC_STD_ADPT_CNTRL | GPIO66 | Out (3.3V) | `gpio_adp_cntrl` | Standard adapter power gate control |
| 29 | I2C_MECC_PDC4_INT_L | GPIO67 | In (1.8V/3.3V) | `gpio_ec_imu_int_l` | Type-C PD Port 4 alert / IMU interrupt |
| **Port 7 Signals** | | | | | |
| 172 | C_IOEX_KBD_INTR | GPIO70 | In (3.3V) | `gpio_ioex_kbd_int_odl` | Discrete keyboard IO expander interrupt |
| 7 | MECC_RSMRST_PWRGD_L | GPIO72 | In (1.8V) | `pwr_rsmrst_pwrgd` | Resume Reset power good (RSMRST_PWRGD_N) |
| 66 | MECC_EN_PP5000_FAN | GPIO73 | Out (3.3V) | `gpio_en_pp5000_fan` | 5V Fan power enable |
| 74 | MECC_TABLET_MODE_L | GPIO74 | In (1.8V) | `gpio_tablet_mode_l` | Tablet mode hall sensor (HALLOUT_SNSR) |
| 131 | MECC_EN_PP5000_S5 | GPIO76 | In/Out (3.3V) | `gpio_en_pp5000_s5` | 5V S5 rail enable |
| **Port 8 Signals** | | | | | |
| 104 | VIRTUAL_BATTERY | GPIO80 | In (3.3V) | `gpio_virtual_battery` | Virtual battery presence switch |
| - | KSO13 | GPIO82 | Out (3.3V) | `aic-pins` | Keyboard scan out 13 |
| - | KSO14 | GPIO83 | Out (3.3V) | `aic-pins` | Keyboard scan out 14 |
| 124 | WC_USBC_PD_DATA_AIC2 | GPIO87 | In/Out / I2C (3.3V) | `i2c1_0_sda` | I2C1 data (USB Type-C PD Port 2) |
| **Port 9 Signals** | | | | | |
| 132 | WC_USBC_PD_CLK_AIC2 | GPIO90 | Out / I2C (3.3V) | `i2c1_0_scl` | I2C1 clock (USB Type-C PD Port 2) |
| 165 | SMB_BS_DATA | GPIO91 | In/Out / I2C (3.3V) | `i2c2_0_sda` | I2C2 data (Battery / Charger / CBI) |
| 160 | SMB_BS_CLK | GPIO92 | Out / I2C (3.3V) | `i2c2_0_scl` | I2C2 clock (Battery / Charger / CBI) |
| 49 | MECC_SLP_S3_L | GPIO94 | In (1.8V) | `gpio_slp_s3_ls` | PCH SLP_S3# sleep status |
| 191 | MECC_RSMRST_L | GPIO95 | Out (1.8V) | `pwr_ec_pch_rsmrst` | Resume Reset output to SoC (PM_RSMRST_N) |
| **Port A Signals** | | | | | |
| 57 | MECC_SOC_PWRBTN_L | GPIOA0 | Out / OD (1.8V) | `gpio_ec_soc_pwr_btn_odl` | PCH Power Button (PM_PWRBTN_N) |
| 119 | CPU_C10_GATE | GPIOA5 | In (1.8V) | `gpio_cpu_c10_gate_l` | CPU C10 gate power status |
| 64 | MECC_SLP_S4_L | GPIOA6 | In (1.8V) | `gpio_slp_s4_ls_l` | PCH SLP_S4# sleep status |
| **Port B Signals** | | | | | |
| 167 | MECC_SLP_S0_L | GPIOB0 | In (1.8V) | `pwr_slp_s0` | PCH SLP_S0# sleep status (PCH_SLP_S0_N) |
| - | KSO16 | GPIOB1 | Out (3.3V) | `aic-pins` | Keyboard scan out 16 |
| 27 | C_USBC_PD_DATA_AIC3_4 | GPIOB2 | In/Out / I2C (3.3V) | `i2c7_0_sda` | I2C7 data (PD Port 3/4) |
| 10 | C_USBC_PD_CLK_AIC3_4 | GPIOB3 | Out / I2C (3.3V) | `i2c7_0_scl` | I2C7 clock (PD Port 3/4) |
| 81 / 86 | SMDAT0 / WC_PD_DATA1 | GPIOB4 | In/Out / I2C (3.3V) | `i2c0_0_sda` | I2C0 data (Target mode / PD Port 1) |
| 79 / 78 | SMCLK0 / WC_PD_CLK1 | GPIOB5 | In/Out / I2C (3.3V) | `i2c0_0_scl` | I2C0 clock (Target mode / PD Port 1) |
| 100 | MECC_EN_EDP_BKLT | GPIOB7 | Out (3.3V) | `gpio_ec_edp_bl_en` | eDP Backlight enable |
| **Port C Signals** | | | | | |
| - | LED_2 | GPIOC0 | Out (3.3V) | `led_2` | Status LED 2 |
| 116 | MECC_SLP_A_L | GPIOC1 | In (1.8V) | `gpio_slp_a_l` | PCH SLP_A# sleep status |
| 83 | KBD_BKLT_CTRL | GPIOC2 | Out / PWM (3.3V) | `gpio_ec_kb_bl_pwm` | Keyboard backlight PWM control |
| 36 | MECC_CPU_FAN_PWM | GPIOC3 | Out / PWM (3.3V) | `pwm0` | CPU Fan PWM drive control |
| - | LED_1 | GPIOC4 | Out (3.3V) | `led_1` | Status LED 1 |
| 134 | MECC_SYS_RST_ODL | GPIOC5 | Out / OD (1.8V) | `pwr_sys_rst` | Platform / SoC System Reset |
| 187 | MECC_EC_PCH_INT_ODL | GPIOC6 | Out / OD (1.8V) | `gpio_ec_soc_int_odl` | EC to PCH host interrupt (MKBP sync) |
| 192 | MECC_WP_ODL | GPIOC7 | In (3.3V) | `gpio_wp_odl` | Flash Write Protect input (active-low) |
| **Port D Signals** | | | | | |
| 151 | I2C_MECC_ISH_INT_L | GPIOD0 | Out / OD (1.8V) | `dsp_comms (int-gpios)` | EC to ISH interrupt / alert |
| 28 | MECC_AMP_MUTE_ODL | GPIOD1 | Out / OD (1.8V) | `dsp_comms (event-gpios)` | Audio Amp Mute control (PDB_CTRL) |
| 108 / 97 | MECC_ACOK / CAT_ERR | GPIOD2 | In (3.3V) | `gpio_acok_od` | Charger AC OK present detect |
| 159 | IO_EXP_INT_MECC | GPIOD3 | In (3.3V) | `gpio_pca_int_l` | Baseboard IO expander interrupt |
| 126 | MECC_EC_BATT_PRES_ODL | GPIOD4 | In (3.3V) | `gpio_ec_batt_pres_odl` | Battery pack detect (BAT_DET) |
| 35 | MECC_SLP_S5_N | GPIOD5 | In (1.8V) | `pwr_slp_s5` | PCH SLP_S5# sleep status |
| 145 | MECC_EC_PP5000_USBA | GPIOD7 | Out (3.3V) | `gpio_en_pp5000_usba` | USB Type-A 5V power enable |
| **Port E Signals** | | | | | |
| 89 | I3C_0_SDA | GPIOE3 | In/Out / I3C (1.8V) | `i3c_0_sda` | I3C Bus 0 Data |
| 90 | I3C_0_SCL | GPIOE4 | Out / I3C (1.8V) | `i3c_0_scl` | I3C Bus 0 Clock |
| 87 | MECC_SLP_SUS_L | GPIOE5 | In (1.8V) | `pwr_slp_sus` | PCH SLP_SUS# sleep status |
| **Port F Signals** | | | | | |
| 6 | I2C_MECC_PDC1_INT_L | GPIOF0 | In (3.3V) | `gpio_usb_pd_int_ls_odl` | Type-C PD Port 1 alert interrupt |
| 181 | VOL_DN_EC | GPIOF1 | In (3.3V) | `gpio_ec_voldn_btn_odl` | Volume Down button |
| 161 | SMB_PCH_DATA | GPIOF2 | In/Out / I2C (1.8V) | `i2c4_1_sda` | PCH SMBUS data / Target I2C4 |
| 156 | SMB_PCH_CK | GPIOF3 | In/Out / I2C (1.8V) | `i2c4_1_scl` | PCH SMBUS clock / Target I2C4 |
| 164 | C_MECC_CSE_EARLY_SW | GPIOF5 | Out / OD (1.8V) | `cse-early-rec-gpios` | CSE Early Recovery switch |

---

## Complete MECC Pin to NPCX GPIO Cross-Reference

| MECC Pin | MECC Signal Name | NPCX GPIO | Direction | Voltage Rail | Function / Role |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **2** | GND | GND | Power | GND | Ground |
| **3** | V3.3A | V3.3A | Power | PP3300_MECC_Z | 3.3V Always-on rail |
| **4** | V3.3A | V3.3A | Power | PP3300_MECC_Z | 3.3V Always-on rail |
| **5** | GND | GND | Power | GND | Ground |
| **6** | WC_USBC_PD_ALRT_AIC1 | GPIOF0 | Input | V3.3_EC_Core | Type-C PD Port 1 alert |
| **7** | RSMRST_PWRGD_N | GPIO72 | Input | +V1P8A | Resume Reset Power Good |
| **8** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **9** | GND | GND | Power | GND | Ground |
| **10** | C_USBC_PD_CLK_AIC3_4 | GPIOB3 (I2C7 SCL) | Bidirectional | V3.3_EC_Core | I2C7 Clock (PD Port 3/4) |
| **11** | GND | GND | Power | GND | Ground |
| **12** | C_CHRG_AMON/BMON | GPIO37 (ADC5) | Input / ADC | V3.3_EC_Core | Charger current monitor |
| **13** | C_KSO_02 | GPIO16 (KSO2) | Output | V3.3_EC_Core | Keyboard scan out 2 |
| **14** | GND | GND | Power | GND | Ground |
| **15** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **16** | GND | GND | Power | GND | Ground |
| **20** | GND | GND | Power | GND | Ground |
| **21** | C_KSI_00 | GPIO31 (KSI0) | Input | V3.3_EC_Core | Keyboard scan in 0 |
| **22** | ALL_SYS_PWRGD | GPIO63 | Input | V3.3S | All System Power Good |
| **23** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **24** | GND | GND | Power | GND | Ground |
| **25** | C_EC_KSI_00 | GPIO31 (KSI0) | Input | V3.3_EC_Core | Keyboard scan in 0 to EC |
| **26** | STD_ADPT_CNTRL_GPIO | GPIO66 | Output | V3.3_EC_Core | Standard adapter power gate |
| **27** | C_USBC_PD_DATA_AIC3_4| GPIOB2 (I2C7 SDA) | Bidirectional | V3.3_EC_Core | I2C7 Data (PD Port 3/4) |
| **28** | C_EC_PDB_CTRL | GPIOD1 | Output / OD | +V1P8A | Audio Amp Mute control |
| **29** | C_USBC_PD_ALRT_AIC4 | GPIO67 | Input | V3.3_EC_Core | Type-C PD Port 4 alert |
| **30** | PCH_PWROK_EC | GPIO61 | Output / OD | +V1P8S | Platform Power OK |
| **32** | GND | GND | Power | GND | Ground |
| **33** | VR_TEMP/V_1P05_PCH_EC | GPIO43 (ADC10)| Input / ADC | V3.3_EC_Core | VR temperature sensor |
| **35** | PM_SLP_S5_N | GPIOD5 | Input | +V1P8A | PCH SLP_S5# sleep status |
| **36** | CPU_PWM_FAN | GPIOC3 (PWM0) | Output / PWM | V3.3_EC_Core | Fan PWM drive control |
| **39** | GND | GND | Power | GND | Ground |
| **41** | PM_BATLOW_N | GPIOA5 | Input | +V1P8A | PCH Battery Low status |
| **42** | CPU_TACHO_FAN | GPIO40 (TA1_1) | Input / Tach | V3.3_EC_Core | Fan tachometer feedback |
| **45** | C_KSI_02_03 | GPIO27 (KSI2) | Input | V3.3_EC_Core | Keyboard scan in 2/3 |
| **46** | C_EC_KSI_02_03 | GPIO27 (KSI2) | Input | V3.3_EC_Core | Keyboard scan in 2/3 to EC |
| **47** | AMBIENT_THERM_IN0 | GPIO45 (ADC0) | Input / ADC | V3.3_EC_Core | Ambient thermistor |
| **48** | BUF_PLT_RST_N | GPIOD2 | Input | +V1P8A | Platform Reset from SoC |
| **49** | PM_SLP_S3_N | GPIO94 | Input | +V1P8A | PCH SLP_S3# sleep status |
| **52** | GND | GND | Power | GND | Ground |
| **55** | EC_DS3 | GPIO41 | Output | V3.3_EC_Core | S5 rails enable (EN_S5_RAILS) |
| **56** | SKIN_THERM_IN | GPIO44 (ADC1) | Input / ADC | V3.3_EC_Core | Skin temperature thermistor |
| **57** | PM_PWRBTN_N | GPIOA0 | Output / OD | +V1P8A | PCH Power Button output |
| **58** | GND | GND | Power | GND | Ground |
| **59** | C_GSC_EC_PWR_BTN_N | GPIO00 | Input | V3.3_EC_Core | Power button input from GSC |
| **60** | +V5A | +V5A | Power | PP5000_MECC_Z | 5V System Power rail |
| **63** | SMC_RST | GPIOD2 | Input | V3.3A_RTC | EC Reset from Dagwood |
| **64** | PM_SLP_S4_N | GPIOA6 | Input | +V1P8A | PCH SLP_S4# sleep status |
| **65** | C_USBC_PD_ALRT_AIC3 | GPIOB2 | Input | V3.3_EC_Core | Type-C PD Port 3 alert |
| **66** | EC_FAN_PWR_DISABLE_N | GPIO73 | Output | V3.3_EC_Core | Fan 5V power enable |
| **67** | GND | GND | Power | GND | Ground |
| **69** | DDR_THERM_IN | GPIO41 (ADC3) | Input / ADC | V3.3_EC_Core | DDR / SoC memory thermistor |
| **70** | GND | GND | Power | GND | Ground |
| **73** | C_ISH_EC_CLK | GPIO33 (I2C5 SCL)| Bidirectional | +V1P8A | I2C5 Clock (EC-ISH interface) |
| **74** | EC_SLATEMODE_HALLOUT_SNSR | GPIO74 | Input | +V1P8A | Tablet mode hall sensor |
| **77** | GND | GND | Power | GND | Ground |
| **78** | WC_USBC_PD_CLK_AIC1 | GPIOB5 (I2C0 SCL)| Bidirectional | V3.3_EC_Core | I2C0 Clock (PD Port 1) |
| **79** | SMCLK0 | GPIOB5 (I2C0 SCL)| Bidirectional | V3.3_EC_Core | Primary Dagwood Target I2C SCL |
| **80** | EC_1.8V_A_rail | +V1P8A | Power | PP1800_MECC_S | 1.8V Switched Power rail |
| **81** | SMDAT0 | GPIOB4 (I2C0 SDA)| Bidirectional | V3.3_EC_Core | Primary Dagwood Target I2C SDA |
| **83** | KBD_BKLT_CTRL | GPIOC2 (PWM) | Output / PWM | V3.3_EC_Core | Keyboard backlight PWM |
| **84** | GND | GND | Power | GND | Ground |
| **86** | WC_USBC_PD_DATA_AIC1 | GPIOB4 (I2C0 SDA)| Bidirectional | V3.3_EC_Core | I2C0 Data (PD Port 1) |
| **87** | D_PM_SLP_SUS_EC_N | GPIOE5 | Input | +V1P8A | PCH SLP_SUS# sleep status |
| **88** | EC_1.8V_A_rail | +V1P8A | Power | PP1800_MECC_S | 1.8V Switched Power rail |
| **89** | I3C_0_SDA | GPIOE3 (I3C0 SDA)| Bidirectional | +V1P8A | I3C Bus 0 Data |
| **90** | I3C_0_SCL | GPIOE4 (I3C0 SCL)| Output | +V1P8A | I3C Bus 0 Clock |
| **94** | GND | GND | Power | GND | Ground |
| **97** | CPU_CAT_ERR | GPIOD2 | Input | V3.3_EC_Core | CPU Catastrophic Error |
| **99** | SMC_WAKE_SCI_N | GPIO32 | Output / OD | V3.3A | EC to PCH Wake SCI |
| **100** | C_EDP_BKLT_EN | GPIOB7 | Output | V3.3_EC_Core | eDP Backlight enable |
| **104** | VIRTUAL_BATTERY | GPIO80 | Input | V3.3A_RTC | Virtual battery present switch |
| **108** | BC_ACOK | GPIOD2 | Input | V3.3A_RTC | Charger AC OK present detect |
| **111** | ESPI_RST_N | GPIO57 (eSPI RST#)| Input | +V1P8A | eSPI Reset (active-low) |
| **112** | GND | GND | Power | GND | Ground |
| **115** | PWRBTN_EC_IN_N | GPIO00 | Input | V3.3A_RTC | Power button input |
| **116** | PM_SLP_A_N | GPIOC1 | Input | +V1P8A | PCH SLP_A# sleep status |
| **118** | WC_USBC_PD_ALRT_AIC2 | GPIO02 | Input | V3.3_EC_Core | Type-C PD Port 2 alert |
| **119** | CPU_C10_GATE | GPIOA5 | Input | +V1P8A | CPU C10 gate power status |
| **123** | SMC_LID | GPIO01 | Input | V3.3A_RTC | Lid open/closed sensor switch |
| **124** | WC_USBC_PD_DATA_AIC2 | GPIO87 (I2C1 SDA)| Bidirectional | V3.3_EC_Core | I2C1 Data (PD Port 2) |
| **126** | BAT_DET | GPIOD4 | Input | V3.3_EC_Core | Battery pack detect |
| **127** | C_ISH_EC_DATA | GPIO36 (I2C5 SDA)| Bidirectional | +V1P8A | I2C5 Data (EC-ISH interface) |
| **131** | EN_PP5000_S5 | GPIO76 | Output | V3.3_EC_Core | 5V S5 rail enable |
| **132** | WC_USBC_PD_CLK_AIC2 | GPIO90 (I2C1 SCL)| Bidirectional | V3.3_EC_Core | I2C1 Clock (PD Port 2) |
| **134** | RST_PUSH_N_D | GPIOC5 | Output / OD | V3.3_EC_Core | System Reset (SYS_RST_ODL) |
| **135** | C_CCD_MODE_ODL | GPIO43 | Input | V3.3_EC_Core | CCD Mode detect |
| **138** | STD_ADP_PRSNT | GPIO62 | Input | V3.3_EC_Core | DC barrel adapter detect |
| **142** | eSPI_ALERT0_N | GPIO47 (eSPI ALERT0#)| Output / OD | +V1P8A | eSPI alert 0 |
| **145** | EN_PP5000_USBA | GPIOD7 | Output | V3.3_EC_Core | USB Type-A 5V power enable |
| **151** | C_EC_ISH_ALRT | GPIOD0 | Output / OD | +V1P8A | EC to ISH interrupt |
| **156** | SMB_PCH_CK | GPIOF3 (I2C4 SCL)| Bidirectional | +V1P8A | PCH SMBUS clock / I2C4 |
| **159** | IO_EXP_INT_MECC | GPIOD3 | Input | V3.3_EC_Core | Baseboard IO expander interrupt |
| **160** | SMB_BS_CLK | GPIO92 (I2C2 SCL)| Bidirectional | V3.3_EC_Core | I2C2 Clock (Battery / Charger) |
| **161** | SMB_PCH_DATA | GPIOF2 (I2C4 SDA)| Bidirectional | +V1P8A | PCH SMBUS data / I2C4 |
| **163** | eSPI_CS0_N | GPIO46 (eSPI CS0#)| Input | +V1P8A | eSPI Chip Select 0 |
| **164** | C_MECC_CSE_EARLY_SW | GPIOF5 | Output / OD | +V1P8A | CSE Early Recovery switch |
| **165** | SMB_BS_DATA | GPIO91 (I2C2 SDA)| Bidirectional | V3.3_EC_Core | I2C2 Data (Battery / Charger) |
| **167** | PCH_SLP_S0_N | GPIOB0 | Input | +V1P8A | PCH SLP_S0# sleep status |
| **169** | PROCHOT_EC_N | GPIO35 | Output / OD | V3.3_EC_Core | Processor Hot alert (PROCHOT#) |
| **172** | C_IOEX_KBD_INTR | GPIO70 | Input | V3.3_EC_Core | Keyboard IO expander interrupt |
| **174** | VOLUME_UP | GPIO50 | Input | V3.3_EC_Core | Volume Up button |
| **177** | ESPI_IO[2] | GPIO53 (eSPI IO2)| Bidirectional | +V1P8A | eSPI Data IO2 |
| **178** | EC_UART_RX | GPIO64 (CR_SIN1)| Input / UART | V3.3_EC_Core | EC Console UART RX (CR_SIN1) |
| **181** | VOL_DN_EC | GPIOF1 | Input | V3.3_EC_Core | Volume Down button |
| **184** | ESPI_IO[1] | GPIO52 (eSPI IO1)| Bidirectional | +V1P8A | eSPI Data IO1 |
| **185** | EC_UART_TX | GPIO65 (CR_SOUT1)| Output / UART| V3.3_EC_Core | EC Console UART TX (CR_SOUT1)|
| **187** | SMC_EXTSMI_N / PCH_INT| GPIOC6 | Output / OD | +V1P8A | EC to PCH host interrupt (MKBP)|
| **190** | ESPI_IO[0] | GPIO51 (eSPI IO0)| Bidirectional | +V1P8A | eSPI Data IO0 |
| **191** | PM_RSMRST_N | GPIO95 | Output | +V1P8A | Resume Reset output to SoC |
| **192** | C_EC_WP_ODL | GPIOC7 | Input | V3.3_EC_Core | Flash Write Protect input |
| **198** | ESPI_IO[3] | GPIO54 (eSPI IO3)| Bidirectional | +V1P8A | eSPI Data IO3 |
| **199** | ESPI_CLK | GPIO55 (eSPI CLK)| Input / eSPI | +V1P8A | eSPI Bus Clock |
