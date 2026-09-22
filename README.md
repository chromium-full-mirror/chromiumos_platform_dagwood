# Dagwood

## Firmware build, flash and debug

The firmware can be built and flashed both with or without the chroot, though
it's recommended reusing the checkout that is part of a ChromiumOS SDK as that
includes the necessary configuration to upload changes to Gerrit.

### Chroot / Docker build and flash

Build the firmware:
```
./build_from_chroot.py
```

Flash the firmware using an attached ST-Link debugger via OpenOCD:
```
# Build and flash:
./build_from_chroot.py -f

# Or flash an existing build without rebuilding:
./build_from_chroot.py --flash-only
```

To flash using USB DFU instead:
```
./build_from_chroot.py -f --flasher dfu
```

You can also run this inside the EC Docker container or directly from the host:
```
./src/platform/ec/docker/run_docker.sh bash -c "cd /workspace/src/platform/dagwood && ./build_from_chroot.py -f"
```

Or flash directly with OpenOCD outside the chroot / container:
```
openocd -f firmware/boards/google/dagwood/support/openocd-stlink.cfg \
  -c "program build/dagwood/build-singleimage/zephyr/zephyr.elf verify reset exit"
```

### Out of chroot build

This setup reuses the upstream Zephyr repository and modules directly, requires
an install of the zephyr-sdk but all the debugging tools are available.

Venv setup (only needed once)

```
sudo apt install python3-venv
python3 -m venv ~/chromiumos/src/platform/dagwood/.venv
```

Enter the venv (once before either setting up or building):
```
source ~/chromiumos/src/platform/dagwood/.venv/bin/activate
```

Initialize the project modules and install dependencies (only needed once):

```
cd ~/chromiumos/src/platform/dagwood
pip install west
unset ZEPHYR_BASE # only needed if there's other Zephyr checkouts in the system
west init -l firmware
west update -o=--depth=1 -n
pip install -U -r zephyr/scripts/requirements.txt
west sdk install -t arm-zephyr-eabi
sudo apt install cmake ninja-build ccache dfu-util
```

Build and flash:

```
cd ~/chromiumos/src/platform/dagwood
west update # only needed if there was a breaking change in the modules
west build firmware
west flash
```

### Build without a ChromiumOS SDK

It's possible to build, flash and debug the firmware without the whole
chromiumos checkout, in that case just pick an arbitrary directory as a
workspace instead of `~/chromiumos/src/platform/dagwood` and initialize
it with `west init --mf firmware/west.yml -m
https://chromium.googlesource.com/chromiumos/platform/dagwood`, the rest
of the process it the same.

This setup takes less disk space, but results in a checkout with no tools to
upload changelists.

### Build with a standalone full Zephyr checkout

The firmware can be built against an existing Zephyr project checkout, in that
case follow the project [Getting Started
Guide](https://docs.zephyrproject.org/latest/getting_started/index.html), make
sure that the `ZEPHYR_BASE` variable is set and run `west build` from the
project `firwmare` directory.

### Available runners

There's multiple options for flashing and debugging the board firmware,
depending on what's hardware is available.

#### USB-DFU

Just for flashing, uses the embedded USB DFU bootloader, requires no additional
hardware. Put the board in DFU mode (press NRST while holding the BOOT0 button)
and then run:

```
west flash -r dfu-util
```

NOTE: flashing with dfu-util normally fails in an error and requires a manual
reset of the board

#### JLink

JLink can be used for both flahing and debugging, requires the [J-Link Software](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) installed.

Flash with:

```
west flash -r jlink
```

or start a debugging session with

```
west debug -r jlink
```

#### ST-LINK

ST-LINK can be used for flashing and debugging with OpenOCD or STM32CubeProgrammer.

Flash directly using OpenOCD:
```
openocd -f firmware/boards/google/dagwood/support/openocd-stlink.cfg \
  -c "program build/dagwood/build-singleimage/zephyr/zephyr.elf verify reset exit"
```

Or using `build_from_chroot.py`:
```
./build_from_chroot.py -f
```

Or using `west`:
```
west flash -r openocd --config firmware/boards/google/dagwood/support/openocd-stlink.cfg
```

Or with STM32CubeProgrammer (requires [host tools](https://docs.zephyrproject.org/latest/develop/flash_debug/host-tools.html#stm32cubeprogrammer-flash-host-tools)):
```
west flash -r stm32cubeprogrammer
```

Start a debugging session with:
```
west debug -r openocd
```

or

```
west debug -r stm32cubeprogrammer
```

#### CMSIS-DAP

CMSIS-DAP can be used for both flashing and debugging with openocd and is part
of Zephyr SDK.

Flash with:

```
west flash -r openocd
```

or start a debugging session with

```
west debug -r openocd
```

## Persistent serial port path

The project uses a pair of standard USB CDC-ACM interfaces for the main board
and AIC board shell. These can be mapped to a persistent path using the
following udev rules (add them on a file in `/etc/udev/rules.d/`):

```
SUBSYSTEM=="tty", ACTION=="add", ATTRS{interface}=="Dagwood shell", SYMLINK+="tty-dw-shell"
SUBSYSTEM=="tty", ACTION=="add", ATTRS{interface}=="EC shell", SYMLINK+="tty-ec-shell"
```

## Signal and Pin Mapping Documentation

Reference pinouts and signal mappings between the 200-pin Intel MECC connector and the Dagwood fixture MCU as well as supported EC Add-In Cards (AICs):

- [MECC to Dagwood MCU Mapping](mecc_to_mcu_mapping.md) - STM32 MCU signal mapping and power rail connections.
- [MECC to ITE IT82002 Mapping](mecc_to_ite_mapping.md) - ITE Tech IT82002 (`ite-aic`) GPIO and peripheral mapping.
- [MECC to Nuvoton NPCX9 Mapping](mecc_to_npcx_mapping.md) - Nuvoton NPCX9 (`npcx-aic`) GPIO and peripheral mapping.
- [MECC to Realtek RTS5912 Mapping](mecc_to_rtk_mapping.md) - Realtek RTS5912 (`rtk-aic`) GPIO and peripheral mapping.
