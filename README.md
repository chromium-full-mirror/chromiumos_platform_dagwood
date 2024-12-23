# EC AIC Tester

## Firmware build, flash and debug

The firmware can be built and flashed both with or without the chroot, though
it's recommended reusing the checkout that is part of a ChromiumOS SDK as that
includes the necessary configuration to upload changes to Gerrit.

### Chroot build

TODO

### Out of chroot build

This setup reuses the repository that is part of the normal chroot setup, just
adds a west workspace on top of it, which can coexist with the normal one
managed with repo.

Venv setup (only needed once)

```
sudo apt install python3-venv
python3 -m venv ~/chromiumos/src/platform/ec-aic-tester/.venv
```

Enter the venv (once before either setting up or building):
```
source ~/chromiumos/src/platform/ec-aic-tester/.venv/bin/activate
```

Initialize the project modules and install dependencies (only needed once):

```
cd ~/chromiumos/src/platform/ec-aic-tester
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
cd ~/chromiumos/src/platform/ec-aic-tester
west update # only needed if there was a breaking change in the modules
west build firmware
west flash
```

### Build without a ChromiumOS SDK

It's possible to build, flash and debug the firmware without the whole
chromiumos checkout, in that case just pick an arbitrary directory as a
workspace instead of `~/chromiumos/src/platform/ec-aic-tester` and initialize
it with `west init --mf firmware/west.yml -m
https://chromium.googlesource.com/chromiumos/platform/ec-aic-tester`, the rest
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

ST-LINK requires the [STM32CubeProgrammer host
tools](https://docs.zephyrproject.org/latest/develop/flash_debug/host-tools.html#stm32cubeprogrammer-flash-host-tools).

Flash with:

```
west flash -r stm32cubeprogrammer
```

or start a debugging session with

```
west debug -r stm32cubeprogrammer
```

TODO: verify that the ones above actually work and check if openocd works for debugging
