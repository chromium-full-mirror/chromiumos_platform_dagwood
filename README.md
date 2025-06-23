# Dagwood

## Firmware build, flash and debug

The firmware can be built and flashed both with or without the chroot, though
it's recommended reusing the checkout that is part of a ChromiumOS SDK as that
includes the necessary configuration to upload changes to Gerrit.

### Chroot build

```
./build_from_chroot.py
```

The script is also used to flash the image by specifying the `-f` flag, only
`dfu-util` is supported. For other flashing tools and source level debugging
use the non chroot west build.

### Out of chroot build

This setup reuses the repository that is part of the normal chroot setup, just
adds a west workspace on top of it, which can coexist with the normal one
managed with repo.

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

### Using the usptream repositories instead of chromium mirrored ones

The project is setup to use the chromium mirror of Zephyr and modules, but can
also be built against the upstream repositories directly. To do that use the
`west-upstream.yml` manifest when initializing the west workspace, for example

```
west init -l firmware --mf west-upstream.yml
```

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

ST-LINK requires the [STM32CubeProgrammer host
tools](https://docs.zephyrproject.org/latest/develop/flash_debug/host-tools.html#stm32cubeprogrammer-flash-host-tools).

Flash with:

```
west flash -r stm32cubeprogrammer
```

or

```
west flash -r openocd --config firmware/boards/google/dagwood/support/openocd-stlink.cfg
```

or start a debugging session with

```
west debug -r stm32cubeprogrammer
```

or

```
west debug -r openocd
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

## Flashing the AIC board

### ITE

```
zmake build ite-aic
itecomdbgr -f build/zephyr/ite-aic/output/ec.bin -d /dev/ttyACM1
# reset the ec (ec reset)
```

### NPCX

Assuming the tester shell is on /dev/ttyACM0 and the EC on /dev/ttyACM1, from
the chroot in the EC directory:

```
zmake build npcx-aic
# power on the ec (ec power on) enter bootloader mode (npcx_boot) on the tester shell
uartupdatetool --port=ttyACM1 --baudrate=115200 --opr=wr --addr=0x200C3020 --file build/zephyr/npcx-aic/output/npcx_monitor.bin
uartupdatetool --port=ttyACM1 --baudrate=115200 --opr=wr --auto --offset=0 --file build/zephyr/npcx-aic/output/ec.bin
# reset the ec (ec reset)
```

## Persistent serial port path

The project uses a pair of standard USB CDC-ACM interfaces for the main board
and AIC board shell. These can be mapped to a persistent path using the
following udev rules (add them on a file in `/etc/udev/rules.d/`):

```
SUBSYSTEM=="tty", ACTION=="add", ATTRS{interface}=="Dagwood shell", SYMLINK+="tty-dw-shell"
SUBSYSTEM=="tty", ACTION=="add", ATTRS{interface}=="EC shell", SYMLINK+="tty-ec-shell"
```
