# MECC to MCU Signal Mapping

| MECC Signal Name | MCU Signal Name | Function / Description |
| :--- | :--- | :--- |
| **Port A Signals** |
| UART_MECC_SOC_RX_AIC_TX | MCU_BUFIO_UART4_TX | EC UART transmit |
| UART_MECC_SOC_TX_AIC_RX | MCU_BUFIO_UART4_RX | EC UART receive |
| MECC_CPU_FAN_PWM | MCU_BUFIO_TIM2_CH3 | EC Fan PWM output |
| MECC_CPU_FAN_TACH | MCU_BUFIO_TIM5_CH4 | EC Fan tachometer input |
| ADC_MECC_AMON | MCU_DAC1_OUT1 | EC ADC input (charger) |
| ADC_MECC_AMB_TEMP | MCU_DAC1_OUT2 | EC ADC input (thermistor) |
| | MCU_ADC1_INP3 | ADC input from jumper J6 |
| | RFU_PA7 | Test Point 16 |
| | RFU_PA8 | Test Point 17 |
| | MCU_VBUS_SENSE | PP5000 VBUS sense input |
| TP38 | MCU_BUFIO_PA10 | Test point |
| | MCU_USB_DN | USB FS DN |
| | MCU_USB_DP | USB FS DP |
| | MCU_DBG_SWDIO | SWD Debug I/O |
| | MCU_DBG_SWCLK | SWD Debug Clock |
| | RFU_PA15 | Test point 18 |
| **Port B Signals** |
| | MCU_PB00_INT | Push button 00 input (header only) |
| | MCU_PB01_INT | Push button 01 input - Used to toggle EC power |
| | MCU_PB02 | PB02 LED |
| | MCU_DBG_SW0 | Debug signal |
| | RFU_PB04 | Test point 19 |
| | RFU_PB05 | Test point 20 |
| | USART1_DBG_RX_MCU_TX | MCU Debug UART transmit |
| | USART1_DBG_TX_MCU_RX | MCU Debug UART receive |
| PP3300_MECC_CORE_X | EC_PP3300_MECC_CORE | EC 3.3V core power |
| PP3300_MECC_Z_X | EC_PP3300_MECC_Z | EC 3.3V Z power |
| PP3300_MECC_S_X | EC_PP3300_MECC_S | EC 3.3V S power |
| PPVAR_MECC_VREF_X | EC_PPVAR_MECC_VREF | EC VREF power |
| PP1800_MECC_Z_X | EC_PP1800_MECC_Z | EC 1.8V Z power |
| PP1800_MECC_S_X | EC_PP1800_MECC_S | EC 1.8V S power |
| PP1800_MECC_Z_X | EC_PP5000_MECC_Z | EC 5V Z power |
| PPVAR_MECC_RTC_X | EC_PPVAR_MECC_RTC | EC RTC power |
| **Port C Signals** |
| MECC_BOOT_STALL | MCU_BUFIO_PC00 | Boot status |
| MECC_CAT_ERR_L | MCU_BUFIO_PC01 | CPU Catastrophic Error |
| MECC_CCD_MODE_L | MCU_BUFIO_PC02 | CCD Mode status |
| MECC_EN_EDP_BKLT | MCU_BUFIO_PC03 | Enable eDP Backlight |
| MECC_EN_PP5000_FAN | MCU_BUFIO_PC04 | Enable 5V Fan |
| MECC_EN_PP5000_S5 | MCU_BUFIO_PC05 | Enable 5V S5 |
| MECC_EN_S5_RAILS | MCU_BUFIO_PC06 | Enable S5 Rails |
| TP36 | MCU_BUFIO_PC07 | Test point |
| MECC_GSC_PWRBTN_L | MCU_BUFIO_PC08 | GSC Power Button |
| MECC_PROCHOT_L | MCU_BUFIO_PC09 | Processor Hot |
| MECC_STD_ADPT_CNTRL | MCU_BUFIO_PC10 | Adapter Control |
| MECC_STD_ADPT_PRSNT | MCU_BUFIO_PC11 | Adapter Present |
| MECC_VOLUME_DN_ODL | MCU_BUFIO_PC12 | Volume Down |
| MECC_VOLUME_UP_ODL | MCU_BUFIO_PC13 | Volume Up |
| MECC_WP_ODL | MCU_BUFIO_PC14 | Write Protect |
| TP37 | MCU_BUFIO_PC15 | Test point |
| **Port D Signals** |
| MECC_CPU_C10_GATE_L | MCU_BUFIO_PD00 | CPU C10 Gate |
| MECC_EC_PCH_INT_ODL | MCU_BUFIO_PD01 | PCH Interrupt |
| MECC_RSMRST_L | MCU_BUFIO_PD02 | Resume Reset |
| MECC_RSMRST_PWRGD_L | MCU_BUFIO_PD03 | Resume Reset Power Good |
| MECC_SLP_A_L | MCU_BUFIO_PD04 | Sleep A |
| MECC_SLP_LAN_L | MCU_BUFIO_PD05 | Sleep LAN |
| MECC_SLP_S0_L | MCU_BUFIO_PD06 | Sleep S0 |
| MECC_SLP_S3_L | MCU_BUFIO_PD07 | Sleep S3 |
| MECC_SLP_S4_L | MCU_BUFIO_PD08 | Sleep S4 |
| MECC_SLP_S5_L | MCU_BUFIO_PD09 | Sleep S5 |
| MECC_SLP_WLAN_L | MCU_BUFIO_PD10 | Sleep WLAN |
| MECC_SOC_PWRBTN_L | MCU_BUFIO_PD11 | SoC Power Button |
| MECC_TABLET_MODE_L | MCU_BUFIO_PD12 | Tablet Mode |
| MECC_BATLOW_L | MCU_BUFIO_PD13 | Battery Low |
| MECC_PLT_RST_L | MCU_BUFIO_PD14 | Platform Reset |
| MECC_SLP_SUS_L | MCU_BUFIO_PD15 | Sleep Suspend |
| **Port E Signals** |
| MECC_ACOK | MCU_BUFIO_PE00 | AC OK |
| MECC_EC_BATT_PRES_ODL | MCU_BUFIO_PE01 | Battery Present |
| MECC_EC_PWRBTN_L | MCUUFIO_PE02 | EC Power Button |
| MECC_EC_RST_L | MCU_BUO_PE03 | EC Reset |
| MECC_EN_SLP_Z | MCU_BUO_PE04 | Enable Sleep Z |
| MECC_LID_OPEN | MCU_BUO_PE05 | Lid Status |
| TP42 | MCU_BUFIO_PE06 Test point |
| TP43 | MCU_BUFIO_PE07 Test point |
| MECC_AMP_MUTE_ODL | MCBUFIO_PE08 | Audio Amp Mute |
| MECC_SOC_REC_SWITCH_ODL | MCU_BUFIO_PE09 | Recovery Switch |
| I2C_MECC_PDC1_INT_L | MCU_BUFIO_PE10 | PDC1 Interrupt |
| I2C_MECC_PDC2_INT_L | MCU_BUFIO_PE11 | PDC2 Interrupt |
| I2C_MECC_PDC3_INT_L | MCU_BUFIO_PE12 | PDC3 Interrupt |
| I2C_MECC_PDC4_INT_L | MCU_BUFIO_PE13 | PDC4 Interrupt |
| I2C_MECC_ISH_INT_L | MCU_BUFIO_PE14 | ISH Interrupt |
| I2C_MECC_PCH_INT_L | MCU_BUFIO_PE15 | PCH Interrupt |
| **Port F Signals** |
| I2C_MECC: PDC1, PDC2, PDC3_PDC4, POWER | I2C2_MCU_MECC_SDA | MCU I2C2 |
| I2C_MECC: PDC1, PDC2, PDC3_PDC4, POWER | I2C2_MCU_MECC_SCL | MCU I2C2 |
| MECC_KSI_00 | MCU_RBOX_KSI_00_INT | KSI 00 Input (GSC signal) |
| | RFU_PF03 | Test point 27 |
| | RFU_PF04 | Test point 28 |
| | RFU_PF05 | Test point 29 |
| ESPI_MECC_IO3 | MCU_BUFIO_QSPI_BK1_IO3 | eSPI IO3 |
| ESPI_MECC_IO2 | MCU_BUFIO_QSPI_BK1_IO2 | eSPI IO2 |
| ESPI_MECC_IO0 | MCU_BUFIO_QSPI_BK1_IO0 | eSPI IO0 |
| ESPI_MECC_IO1 | MCU_BUFIO_QSPI_BK1_IO1 | eSPI IO1 |
| ESPI_MECC_CLK | MCU_BUFIO_QSPI_CLK | eSPI Clock |
| ESPI_MECC_RESET_L | MCU_BUFIO_PF11 | eSPI Reset |
| | I2C4_MCU_MUX_A0 | MCU I2C4 mux select |
| | I2C4_MCU_MUX_EN | MCU I2C4 mux enable |
| I2C_MECC: ISH, PCH | I2C4_MCU_MECC_SDA | MCU I2C4 |
| I2C_MECC: ISH, PCH | I2C4_MCU_MECC_SCL | MCU I2C4 |
| **Port G Signals** |
| ESPI_MECC_CS1_L | MCU_BUFIO_PG00 | eSPI Chip Select 1 |
| ESPI_MECC_CS2_L | MCU_BUFIO_PG01 | eSPI Chip Select 2 |
| ESPI_MECC_CS3_L | MCU_BUFIO_PG02 | eSPI Chip Select 3 |
| ESPI_MECC_ALERT0_L | MCU_BUFIO_PG03_INT | eSPI Alert 0 |
| ESPI_MECC_ALERT1_L | MCU_BUFIO_PG04_INT | eSPI Alert 1 |
| ESPI_MECC_ALERT2_L | MCU_BUFIO_PG05_INT | eSPI Alert 2 |
|  **ESPI_MECC_CS0_L | MCU_BUFIO_QSPI_BK1_NCS | eSPI Chip Select 0 |
| ESPI_MECC_ALERT3_L | MCU_BUFIO_PG07_INT | eSPI Alert 3 |
| SPI_MECC_CS0_L | MCU_BUFIO_SPI6_NSS | SPI Chip Select 0 |
| SPI_MECC_CS1_L | MCU_BUFIO_PG09_INT | SPI Chip Select 1 |
| SPI_MECC_FLASH_OE_L | M_BUFIO_PG10_INT | SPI Flash Output Enable |
| | RFU_PG11 | Test point 25 |
| SPI_MECC_SOC_DI_AIC_DO | MCU_BUFIO_SPI6_MISO | SPI Data In (SoC side) |
| SPI_MECC_CLK | MCU_BUFIO_SPI6_SCK | SPI Clock |
| SPI_MECC_SOC_DO_AIC_DI | MCU_BUFIO_SPI6_MOSI | SPI Data Out (SoC side) |
| | RFU_PG15 | Test point 26 |
| **Port H Signals** |
| | MCU_OSC_IN | |
| | MCU_OSC_OUT | |
| MECC_KBD_PWR_BTN_ODL | MCU_KBD_PWR_BTN | Keyboard power button |
| | MCU_KBD_SCANOUT_S | |
| | MCU_KBD_SCANOUT_T | |
| | RFU_PH05 | Test point 22 |
| | I2C3_MCU_RST_L | Test point 30 |
| | I2C3_MUC_SCL | I2C3 SCL |
| | I2C3_MCU_SDA | I2C3 SDA |
| | I2C3_MCU_INT_L| I2C3 interrupt, Test point 33  |
| MECC_KSO_02| MCU_RBOX_KSO_02 | Keyboard output 2 |
| | RFU_PH11 | Test point 23 |
| | RFU_PH12 | Test point 24 |
| MECC_KSI_02_03 | MCU_RBOX_EC_KSI_02_03_INT | KSI 02/03 Input (GSC signal) |
| MECC_EC_KSO_00 | MCU_RBOX_EC_KSI_00 | KSI 00 Input to EC |
| MECC_EC_KSO_02_03 | MCU_RBOX_EC_KSI_02_03 | KSI 02/03 Input to EC |
| **Port I Signals** |
| MECC_PLT_PWROK | MCU_BUFIO_PI00 | Platform Power OK |
| MECC_SYS_PWROK | MCU_BUFIO_PI01 | System Power OK |
| TP34 | MCU_BUFIO_PI02 | Test point |
| TP35 | MCU_BUFIO_PI03 | Test point |
| MECC_EC_PCH_WAKE_ODL | MCU_BUFIO_PI04 | EC to PCH Wake |
| MECC_EC_PP5000_USBA | MCU_BUFIO_PI05 | Enable PP5000 USB A |
| TP39 | MCU_BUFIO_PI06 | Test point |
| MECC_SYS_RST_ODL | MCU_BUFIO_PI07 | AP System Reset |
| MECC_ALL_SYS_PWRGD | MCU_BUFIO_PI08 | All System Power Good |
| MECC_EC_KSO_02_INV | MCU_RBOX_EC_KSO_02_INV_INT | KSO 02 Input (GSC signal) |
| TP40 | MCU_BUFIO_PI10 | Test point |
| TP41 | MCU_BUFIO_PI11 | Test point |
| | I2C2_MCU_MUX_A1 | MCU I2C2 mux select |
| | I2C2_MCU_MUX_A0 | MCU I2C2 mux select |
| | I2C2_MCU_MUX_EN | MCU I2C2 mux enable |
| | RFU_PI15 | Test point 21 |
| **Port J Signals** |
| | | Keyboard scanout signals |
| **Port K Signals** |
| | | Keyboard scanin signals |
