# MECC to Realtek RTS5912 Signal Mapping

This document provides the mapping between the 200-pin **MECC** (Modular
Embedded Controller Card) interface and the GPIO/peripheral pin assignments for
the **Realtek RTS5912** EC Add-In Card (**`rtk-aic`**).

## Overview

- **EC Architecture**: Realtek RTS5912 (RTS5912GR / RTS5912GQ)
- **Reference AIC Board**: `rtk-aic` (`zephyr/program/ec-aic/rtk-aic`)
- **Reference RVP Board**: `ocelotrvp-rtk` (`zephyr/program/ocelot/ocelotrvp-rtk`)
- **Supported Standards**: Intel MECC Specification Revision 2.1

---

## Signal Mapping Grouped by Realtek Port

| MECC Pin | MECC Signal Name | GPIO | Direction / Level | Devicetree Node / Net Name | Function / Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Port a Signals** | | | | | |
| 99 | SMC_WAKE_SCI_N | GPIO000 | Out / OD (3.3V) | `smc-wake-sci-n-3v3-od` | EC to PCH Wake SCI |
| 26 | STD_ADPT_CNTRL | GPIO001 | Out (3.3V) | `std-adpt-cntrl-gpio-3v3-pp` | Standard DC adapter power control gate |
| 163 | eSPI_CS0_N | GPIO004 (espi_cs) | In (1.8V) | `espi_cs_gpio004` | eSPI Chip Select 0 |
| 198 | ESPI_IO[3] | GPIO005 (espi_io3) | In/Out / eSPI (1.8V) | `espi_io3_gpio005` | eSPI Data IO3 |
| 177 | ESPI_IO[2] | GPIO006 (espi_io2) | In/Out / eSPI (1.8V) | `espi_io2_gpio006` | eSPI Data IO2 |
| 184 | ESPI_IO[1] | GPIO007 (espi_io1) | In/Out / eSPI (1.8V) | `espi_io1_gpio007` | eSPI Data IO1 |
| 190 | ESPI_IO[0] | GPIO008 (espi_io0) | In/Out / eSPI (1.8V) | `espi_io0_gpio008` | eSPI Data IO0 |
| 199 | ESPI_CLK | GPIO009 (espi_clk) | In / eSPI (1.8V) | `espi_clk_gpio009` | eSPI Bus Clock |
| 10 | C_USBC_PD_CLK_AIC3_4 | GPIO014 (i2c_2) | In/Out / I2C (3.3V) | `i2c_2 (scl)` | Type-C PD Port 3/4 I2C clock |
| 164 | C_MECC_CSE_EARLY_SW | GPIO015 | Out / OD (1.8V) | `cse_early_rec_sw` / `c-mecc-cse-early-sw-1v8-od` | CSE Early Recovery switch |
| **Port b Signals** | | | | | |
| 156 | SMB_PCH_CK | GPIO017 (i2c_4) | In/Out / I2C (1.8V) | `i2c_4 (scl)` | PCH SMBus / I2C clock |
| 161 | SMB_PCH_DATA | GPIO018 (i2c_4) | In/Out / I2C (1.8V) | `i2c_4 (sda)` | PCH SMBus / I2C data |
| 36 | CPU_PWM_FAN | GPIO025 (pwm2) | Out / PWM (3.3V) | `pwm2` (`pwm2_gpio025`) | CPU Fan PWM drive control |
| 181 | VOL_DN_EC | GPIO029 | In (3.3V) | `gpio_voldn` / `vol-dn-ec-3v3-i` | Volume Down button input (active-low) |
| 192 | C_EC_WP_ODL | GPIO030 | In (1.8V/3.3V) | `gpio_ec_wp_odl` / `ec_wp_odl` | Flash write protect input (active-low) |
| 119 | CPU_C10_GATE | GPIO031 | In (1.8V) | `cpu-c10-gate-1v8-i` | CPU C10 gate power status |
| **Port c Signals** | | | | | |
| 13 | C_KSO_02 | GPIO043 (kso2) | Out (3.3V) | `kso2_gpio043` | Keyboard scan out 2 |
| **Port d Signals** | | | | | |
| 42 | CPU_TACHO_FAN | GPIO052 (tach0) | In / Tach (3.3V) | `tach0` (`tacho0_gpio052`) | CPU Fan tachometer feedback input |
| 55 | EC_DS3 | GPIO053 | Out (3.3V) | `gpio_en_pp3300` / `pwr_en_pp3300_a` / `ec-ds3-3v3-pp` | S5 rails / IMVP VR enable control |
| **Port e Signals** | | | | | |
| 21 / 25 | C_KSI_00 / C_EC_KSI_00 | GPIO064 (ksi0) | In (3.3V) | `ksi0_gpio064` | Keyboard scan in 0 |
| 45 | C_KSI_02_03 | GPIO066 (ksi2) | In (3.3V) | `ksi2_gpio066` | Keyboard scan in 2/3 |
| 33 | VR_TEMP/V_1P05_PCH_EC | GPIO074 (adc0) | In / ADC (3.3V) | `adc0_gpio074` | VR temperature sensor |
| 47 | AMBIENT_THERM_IN0 | GPIO075 (adc1) | In / ADC (3.3V) | `adc1_gpio075` | Ambient thermistor |
| 12 | C_CHRG_AMON/BMON | GPIO076 (adc2) | In / ADC (3.3V) | `adc2_gpio076` | Charger current monitor |
| 56 | SKIN_THERM_IN | GPIO077 (adc3) | In / ADC (3.3V) | `adc3_gpio077` | Skin temperature thermistor |
| 74 | TABLET_MODE_L | GPIO078 | In (1.8V) | `gpio_tablet_mode_l` / `ec-slatemode-hallout-snsr-1v8-i` | Tablet / Clamshell hall sensor |
| **Port f Signals** | | | | | |
| 69 | DDR_THERM_IN | GPIO080 (adc6) | In / ADC (3.3V) | `adc6_gpio080` | DDR / SoC memory thermistor |
| 174 | VOLUME_UP | GPIO083 | In (3.3V) | `gpio_volup` / `volume-up-3v3-i` | Volume Up button input (active-low) |
| 159 | IO_EXP_INT_MECC | GPIO085 | In (3.3V) | `it8801_mfd (irq)` | Baseboard IO expander interrupt |
| 138 | STD_ADP_PRSNT | GPIO093 | In (3.3V) | `gpio_dc_jack_present` / `std-adp-prsnt-3v3-i` | DC barrel adapter present detect |
| 79 / 160 | SMCLK0 / SMB_BS_CLK | GPIO094 (i2c_0) | In/Out / I2C (3.3V) | `i2c_0 (scl)` | Primary Dagwood Target / Charger I2C SCL |
| 81 / 165 | SMDAT0 / SMB_BS_DATA | GPIO095 (i2c_0) | In/Out / I2C (3.3V) | `i2c_0 (sda)` | Primary Dagwood Target / Charger I2C SDA |
| **Port g Signals** | | | | | |
| 28 | C_EC_PDB_CTRL | GPIO096 | In / OD (1.8V) | `c-ec-pdb-ctrl-1v8-od` | PDB control / Audio Amp Mute |
| 22 | ALL_SYS_PWRGD | GPIO099 | Input (3.3V) | `all_sys_pwrgd_i` / `pwr_all_sys_pwrgd` | All System Power Good indicator |
| 78 | WC_USBC_PD_CLK_AIC1 | GPIO100 (i2c_3) | In/Out / I2C (3.3V) | `i2c_3 (scl)` | Type-C PD Port 1 I2C clock |
| 86 | WC_USBC_PD_DATA_AIC1 | GPIO101 (i2c_3) | In/Out / I2C (3.3V) | `i2c_3 (sda)` | Type-C PD Port 1 I2C data |
| 103 / 134 | RST_PUSH_N_D | GPIO103 | Out / OD (1.8V/3.3V) | `pwr_sys_rst` | System Reset output (SYS_RST_ODL) |
| 7 | RSMRST_PWRGD_N | GPIO104 | In (1.8V) | `gpio_rsmrst_pwrgd` / `rsmrst-pwrgd-n-1v8-i` | Resume Reset Power Good |
| 105 | SYS_PWROK_EC | GPIO109 | Out / OD (1.8V) | `sys-pwrok-ec-1v8-od` | System Power OK output |
| 111 | PM_RSMRST_N | GPIO111 | Out (1.8V) | `gpio_ec_pch_rsmrst` / `pm-rsmrst-n-1v8-pp` | Resume Reset output to PCH |
| **Port h Signals** | | | | | |
| 108 | BC_ACOK | GPIO112 | In (3.3V) | `gpio_acok` / `bc-acok-3v3-i` | Charger AC OK present detect |
| 178 | EC_UART_RX | GPIO113 (uart0_rx) | In / UART (3.3V) | `uart0_rx_gpio113` | EC serial console UART RX |
| 185 | EC_UART_TX | GPIO114 (uart0_tx) | Out / UART (3.3V) | `uart0_tx_gpio114` | EC serial console UART TX |
| 115 | PWRBTN_EC_IN_N | GPIO115 | In (3.3V) | `gpio_pwrbtn` / `pwrbtn-ec-in-n-3v3-i` | Power button input (active-low) |
| 117 / 123 | SMC_LID | GPIO117 | In (3.3V) | `gpio_lid` / `smc-lid-3v3-i` | Lid open / closed switch detect |
| - | I2C1_CLK | GPIO118 (i2c_1) | In/Out / I2C (3.3V) | `i2c_1 (scl)` | Secondary I2C controller 1 clock |
| - | I2C1_DATA | GPIO119 (i2c_1) | In/Out / I2C (3.3V) | `i2c_1 (sda)` | Secondary I2C controller 1 data |
| 27 | C_USBC_PD_DATA_AIC3_4 | GPIO121 (i2c_2) | In/Out / I2C (3.3V) | `i2c_2 (sda)` | Type-C PD Port 3/4 I2C data |
| 169 | PROCHOT_EC_N | GPIO122 | Out / OD (3.3V) | `prochot-ec-n-3v3-od` | Processor Hot alert (PROCHOT#) |
| 145 | C_EN_PP5000_TYPEA | GPIO123 | In/Out (3.3V) | `c-en-pp5000-typea-3v3-pp` | USB Type-A 5V power enable |
| 57 | PM_PWRBTN_N | GPIO124 | Out / OD (1.8V) | `pm-pwrbtn-n-1v8-od` | PCH Power Button output |
| 167 | PCH_SLP_S0_N | GPIO125 | In (1.8V) | `gpio_slp_s0` / `pch-slp-s0-n-1v8-i` | PCH SLP_S0# sleep status |
| 126 | BAT_DET | GPIO126 | In (3.3V) | `bat-det-3v3-i` | Battery pack detect |
| 30 | PCH_PWROK_EC | GPIO127 | In/Out / OD (1.8V) | `gpio_pch_pwrok_ec` / `pch-pwrok-ec-1v8-od` | Platform Power OK |
| **Port i Signals** | | | | | |
| 132 | WC_USBC_PD_CLK_AIC2 | GPIO128 (i2c_5) | In/Out / I2C (3.3V) | `i2c_5 (scl)` | Type-C PD Port 2 I2C clock |
| 124 | WC_USBC_PD_DATA_AIC2 | GPIO130 (i2c_5) | In/Out / I2C (3.3V) | `i2c_5 (sda)` | Type-C PD Port 2 I2C data |
| 61 | EC_PG3_WAKE | GPIO131 | Out (3.3V) | `gpio_ec_pch_wake_odl` / `ec-pg3-wake-3v3-pp` | Latch enable / wake control |

---

## Complete MECC Pinout to Realtek RTS5912 Signal Cross-Reference

| Pin # | MECC Signal Name | GPIO / Signal | Direction | Power Rail | Description |
| :---: | :--- | :--- | :--- | :--- | :--- |
| **2** | GND | GND | Power | GND | Ground |
| **3** | V3.3A | V3.3A | Power | PP3300_MECC_Z | 3.3V Always-on rail |
| **4** | V3.3A | V3.3A | Power | PP3300_MECC_Z | 3.3V Always-on rail |
| **5** | GND | GND | Power | GND | Ground |
| **6** | WC_USBC_PD_ALRT_AIC1 | - | Input | V3.3_EC_Core | Type-C PD Port 1 alert |
| **7** | RSMRST_PWRGD_N | GPIO104 | Input | +V1P8A | Resume Reset Power Good |
| **8** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **9** | GND | GND | Power | GND | Ground |
| **10** | C_USBC_PD_CLK_AIC3_4 | GPIO014 (i2c_2) | Bidirectional | V3.3_EC_Core | I2C2 Clock (PD Port 3/4) |
| **11** | GND | GND | Power | GND | Ground |
| **12** | C_CHRG_AMON/BMON | GPIO076 (adc2) | Input / ADC | V3.3_EC_Core | Charger current monitor |
| **13** | C_KSO_02 | GPIO043 (kso2) | Output | V3.3_EC_Core | Keyboard scan out 2 |
| **14** | GND | GND | Power | GND | Ground |
| **15** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **16** | GND | GND | Power | GND | Ground |
| **20** | GND | GND | Power | GND | Ground |
| **21** | C_KSI_00 | GPIO064 (ksi0) | Input | V3.3_EC_Core | Keyboard scan in 0 |
| **22** | ALL_SYS_PWRGD | GPIO099 | Input | V3.3S | All System Power Good |
| **23** | V3.3_EC_Core | V3.3_EC_Core | Power | PP3300_MECC_CORE | 3.3V Core supply to EC |
| **24** | GND | GND | Power | GND | Ground |
| **25** | C_EC_KSI_00 | GPIO064 (ksi0) | Input | V3.3_EC_Core | Keyboard scan in 0 to EC |
| **26** | STD_ADPT_CNTRL_GPIO | GPIO001 | Output | V3.3_EC_Core | Standard adapter power gate |
| **27** | C_USBC_PD_DATA_AIC3_4| GPIO121 (i2c_2) | Bidirectional | V3.3_EC_Core | I2C2 Data (PD Port 3/4) |
| **28** | C_EC_PDB_CTRL | GPIO096 | Output / OD | +V1P8A | Audio Amp Mute control |
| **30** | PCH_PWROK_EC | GPIO127 | Bidirectional / OD | +V1P8S | Platform Power OK |
| **32** | GND | GND | Power | GND | Ground |
| **33** | VR_TEMP/V_1P05_PCH_EC | GPIO074 (adc0) | Input / ADC | V3.3_EC_Core | VR temperature sensor |
| **36** | CPU_PWM_FAN | GPIO025 (pwm2) | Output / PWM | V3.3_EC_Core | Fan PWM drive control |
| **39** | GND | GND | Power | GND | Ground |
| **42** | CPU_TACHO_FAN | GPIO052 (tach0) | Input / Tach | V3.3_EC_Core | Fan tachometer feedback |
| **45** | C_KSI_02_03 | GPIO066 (ksi2) | Input | V3.3_EC_Core | Keyboard scan in 2/3 |
| **47** | AMBIENT_THERM_IN0 | GPIO075 (adc1) | Input / ADC | V3.3_EC_Core | Ambient thermistor |
| **52** | GND | GND | Power | GND | Ground |
| **55** | EC_DS3 | GPIO053 | Output | V3.3_EC_Core | S5 rails enable (EN_S5_RAILS) |
| **56** | SKIN_THERM_IN | GPIO077 (adc3) | Input / ADC | V3.3_EC_Core | Skin temperature thermistor |
| **57** | PM_PWRBTN_N | GPIO124 | Output / OD | +V1P8A | PCH Power Button output |
| **58** | GND | GND | Power | GND | Ground |
| **59** | C_GSC_EC_PWR_BTN_N | GPIO115 | Input | V3.3_EC_Core | Power button input from GSC |
| **60** | +V5A | +V5A | Power | PP5000_MECC_Z | 5V System Power rail |
| **61** | EC_PG3_WAKE | GPIO131 | Output | V3.3A_RTC | Latch enable (EN_SLP_Z) |
| **66** | EC_FAN_PWR_DISABLE_N | - | Output | V3.3_EC_Core | Fan 5V power gate control |
| **67** | GND | GND | Power | GND | Ground |
| **69** | DDR_THERM_IN | GPIO080 (adc6) | Input / ADC | V3.3_EC_Core | DDR / SoC memory thermistor |
| **70** | GND | GND | Power | GND | Ground |
| **74** | EC_SLATEMODE_HALLOUT_SNSR | GPIO078 | Input | +V1P8A | Tablet mode hall sensor |
| **77** | GND | GND | Power | GND | Ground |
| **78** | WC_USBC_PD_CLK_AIC1 | GPIO100 (i2c_3) | Bidirectional | V3.3_EC_Core | I2C3 Clock (PD Port 1) |
| **79** | SMCLK0 | GPIO094 (i2c_0) | Bidirectional | V3.3_EC_Core | Primary Dagwood Target I2C SCL |
| **80** | EC_1.8V_A_rail | +V1P8A | Power | PP1800_MECC_S | 1.8V Switched Power rail |
| **81** | SMDAT0 | GPIO095 (i2c_0) | Bidirectional | V3.3_EC_Core | Primary Dagwood Target I2C SDA |
| **84** | GND | GND | Power | GND | Ground |
| **86** | WC_USBC_PD_DATA_AIC1 | GPIO101 (i2c_3) | Bidirectional | V3.3_EC_Core | I2C3 Data (PD Port 1) |
| **88** | EC_1.8V_A_rail | +V1P8A | Power | PP1800_MECC_S | 1.8V Switched Power rail |
| **94** | GND | GND | Power | GND | Ground |
| **96** | GND | GND | Power | GND | Ground |
| **99** | SMC_WAKE_SCI_N | GPIO000 | Output / OD | V3.3A | EC to PCH Wake SCI |
| **103** | RST_PUSH_N_D | GPIO103 | Output / OD | +V1P8A | System Reset (SYS_RST_ODL) |
| **105** | SYS_PWROK_EC | GPIO109 | Output / OD | +V1P8S | System Power OK |
| **108** | BC_ACOK | GPIO112 | Input | V3.3A_RTC | Charger AC OK present detect |
| **111** | PM_RSMRST_N | GPIO111 | Output | +V1P8A | Resume Reset output to PCH |
| **112** | GND | GND | Power | GND | Ground |
| **115** | PWRBTN_EC_IN_N | GPIO115 | Input | V3.3A_RTC | Power button input |
| **117** | SMC_LID | GPIO117 | Input | V3.3A_RTC | Lid open/closed switch detect |
| **119** | CPU_C10_GATE | GPIO031 | Input | +V1P8A | CPU C10 gate power status |
| **123** | SMC_LID | GPIO117 | Input | V3.3A_RTC | Lid open/closed switch detect |
| **124** | WC_USBC_PD_DATA_AIC2 | GPIO130 (i2c_5) | Bidirectional | V3.3_EC_Core | I2C5 Data (PD Port 2) |
| **126** | BAT_DET | GPIO126 | Input | V3.3_EC_Core | Battery pack detect |
| **132** | WC_USBC_PD_CLK_AIC2 | GPIO128 (i2c_5) | Bidirectional | V3.3_EC_Core | I2C5 Clock (PD Port 2) |
| **134** | RST_PUSH_N_D | GPIO103 | Output / OD | V3.3_EC_Core | System Reset (SYS_RST_ODL) |
| **138** | STD_ADP_PRSNT | GPIO093 | Input | V3.3_EC_Core | DC barrel adapter detect |
| **145** | EN_PP5000_USBA | GPIO123 | Output | V3.3_EC_Core | USB Type-A 5V power enable |
| **156** | SMB_PCH_CK | GPIO017 (i2c_4) | Bidirectional | +V1P8A | PCH SMBUS clock |
| **159** | IO_EXP_INT_MECC | GPIO085 | Input | V3.3_EC_Core | Baseboard IO expander interrupt |
| **160** | SMB_BS_CLK | GPIO094 (i2c_0) | Bidirectional | V3.3_EC_Core | I2C0 Clock (Battery / Charger) |
| **161** | SMB_PCH_DATA | GPIO018 (i2c_4) | Bidirectional | +V1P8A | PCH SMBUS data |
| **163** | eSPI_CS0_N | GPIO004 (espi_cs) | Input | +V1P8A | eSPI Chip Select 0 |
| **164** | C_MECC_CSE_EARLY_SW | GPIO015 | Output / OD | +V1P8A | CSE Early Recovery switch |
| **165** | SMB_BS_DATA | GPIO095 (i2c_0) | Bidirectional | V3.3_EC_Core | I2C0 Data (Battery / Charger) |
| **167** | PCH_SLP_S0_N | GPIO125 | Input | +V1P8A | PCH SLP_S0# sleep status |
| **169** | PROCHOT_EC_N | GPIO122 | Output / OD | V3.3_EC_Core | Processor Hot alert (PROCHOT#) |
| **174** | VOLUME_UP | GPIO083 | Input | V3.3_EC_Core | Volume Up button |
| **177** | ESPI_IO[2] | GPIO006 (espi_io2) | Bidirectional | +V1P8A | eSPI Data IO2 |
| **178** | EC_UART_RX | GPIO113 (uart0_rx) | Input / UART | V3.3_EC_Core | EC Console UART RX |
| **181** | VOL_DN_EC | GPIO029 | Input | V3.3_EC_Core | Volume Down button |
| **184** | ESPI_IO[1] | GPIO007 (espi_io1) | Bidirectional | +V1P8A | eSPI Data IO1 |
| **185** | EC_UART_TX | GPIO114 (uart0_tx) | Output / UART | V3.3_EC_Core | EC Console UART TX |
| **190** | ESPI_IO[0] | GPIO008 (espi_io0) | Bidirectional | +V1P8A | eSPI Data IO0 |
| **191** | PM_RSMRST_N | GPIO111 | Output | +V1P8A | Resume Reset output to SoC |
| **192** | C_EC_WP_ODL | GPIO030 | Input | V3.3_EC_Core | Flash Write Protect input |
| **198** | ESPI_IO[3] | GPIO005 (espi_io3) | Bidirectional | +V1P8A | eSPI Data IO3 |
| **199** | ESPI_CLK | GPIO009 (espi_clk) | Input / eSPI | +V1P8A | eSPI Bus Clock |
