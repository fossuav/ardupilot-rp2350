# ArduPilot for Raspberry Pi RP2350

<!-- markdownlint-disable-next-line MD033 -->
<a href="https://www.raspberrypi.com"><img src="https://www.raspberrypi.com/app/uploads/2022/02/COLOUR-Raspberry-Pi-Symbol-Registered.png" alt="Raspberry Pi" width="100" align="right"></a>

The canonical ArduPilot README can be found [here](https://github.com/ArduPilot/ardupilot/blob/master/README.md)

![Raspberry Pi UAV Flight Controller](libraries/AP_HAL_ChibiOS/hwdef/RPI_UAVFC/RPI_UAVFC_Top.jpg)

ArduPilot is the most advanced, full-featured, and reliable open source autopilot software available.
It has been under development since 2010 by a diverse team of professional engineers, computer scientists, and community contributors.
The autopilot software is capable of controlling almost any vehicle system imaginable, from conventional airplanes, quad planes, multi-rotors, and helicopters to rovers, boats, balance bots, and even submarines.

This repository carries ArduPilot for flight controllers built on the Raspberry Pi RP2350 microcontroller. The port is under review upstream as [ArduPilot/ardupilot#32995](https://github.com/ArduPilot/ardupilot/pull/32995). It adds a new MCU family to ArduPilot's ChibiOS HAL, so it will take time to merge, and until it does ArduPilot's firmware server does not build RP2350 boards. This repository is where to get RP2350 firmware in the meantime, and where the fixes found while flight-testing the port are collected. The intent is:

- To track ArduPilot master (this branch is 4.8.0-dev) with the RP2350 port on top
- For everything here to be open source and headed upstream: each change is an open pull request to ArduPilot, or is on its way into one, as listed under [What Is Included](#what-is-included)
- To publish prebuilt firmware for the boards that have flown, so you do not need a build environment to fly
- To document building, flashing, and what the port does not do yet

## Contents

- [Status](#status)
- [Supported Boards](#supported-boards)
- [Downloading Firmware](#downloading-firmware)
- [Flashing](#flashing)
- [Building from Source](#building-from-source)
- [What Is Included](#what-is-included)
- [Known Limitations](#known-limitations)
- [Reporting Problems](#reporting-problems)

## Status

This is development firmware, and only Copter has flown. RPI_UAVFC has flown on two 5 inch quads in Stabilize, Loiter and Acro, including inverted, on bidirectional DShot600 with a per-motor RPM notch. RTL, Auto and the GPS failsafe have not been flown on any RP2350 board, and no AUTOTUNE has completed: the gains flown so far were tuned by hand. Read [Known Limitations](#known-limitations) before you fly.

## Supported Boards

| Board | MCU | What it is | Prebuilt firmware |
|-------|-----|------------|-------------------|
| [RPI_UAVFC](libraries/AP_HAL_ChibiOS/hwdef/RPI_UAVFC/README.md) | RP2350B | The Raspberry Pi UAV Flight Controller, and the board that flies | Copter |
| [Pico2](libraries/AP_HAL_ChibiOS/hwdef/Pico2/README.md) | RP2350A | A stock Raspberry Pi Pico 2 or Pico 2 W running Simulation on Hardware: the flight code runs on the real chip against a physics model, so nothing need be attached but USB | Copter |
| [Laurel](libraries/AP_HAL_ChibiOS/hwdef/Laurel/README.md) | RP2350B | The first RP2350B development board; nothing has flown on it with the current code | Build from source |

Each board's README covers its pinout, serial ports and settings. All three run both RP2350 cores at 225 MHz: the main loop runs on core 0, and the fast rate thread on core 1 (on by default on RPI_UAVFC and Laurel, off on Pico2).

Plane, Rover and the other vehicles build for these boards but have not been run on them.

## Downloading Firmware

Prebuilt firmware is on the [Releases](https://github.com/fossuav/ardupilot-rp2350/releases/latest) page. Each file name starts with the board it is for:

| File | Use it to |
|------|-----------|
| `<board>_bl.uf2` | Install the ArduPilot bootloader through BOOTSEL, the first time or to recover a board |
| `<board>_arducopter.apj` | Install or update ArduPilot through the ArduPilot bootloader, with `uploader.py` or a ground station |
| `<board>_arducopter.uf2` | Install or update ArduPilot through BOOTSEL, once the ArduPilot bootloader is installed |
| `<board>_bl.bin`, `<board>_arducopter.bin` | Write the bootloader and firmware over SWD |
| `<board>_arducopter_with_bl.hex` | Write both over SWD in one go; this also erases the stored parameters |
| `<board>_arducopter.elf` | Read a crash dump or debug over SWD |
| `uploader.py` | Load an `.apj` over USB from the command line |
| `SHA256SUMS` | Check the downloads |

The file names do not change between releases, so `https://github.com/fossuav/ardupilot-rp2350/releases/latest/download/RPI_UAVFC_arducopter.apj` always fetches the latest firmware for RPI_UAVFC.

## Flashing

The 4 MB of flash on an RP2350 board holds three things: the ArduPilot bootloader, the stored parameters, and the ArduPilot firmware. You install the ArduPilot bootloader once, through the RP2350's own USB bootloader (BOOTSEL), which is in the chip's ROM and cannot be overwritten, so a board can always be recovered. After that, firmware updates go through the ArduPilot bootloader over USB and keep your parameters.

### 1. Put the board in BOOTSEL mode

In BOOTSEL mode the RP2350 appears on your computer as a USB drive named `RP2350`.

- Pico 2: hold the BOOTSEL button while you plug in USB.
- Laurel: hold BOOT and press RESET.
- RPI_UAVFC running Betaflight: connect with Betaflight Configurator, open the CLI tab and enter `bl`.
- Any board whose flash is empty enters BOOTSEL by itself when it is powered.

### 2. Install the ArduPilot bootloader

Copy `<board>_bl.uf2` onto the `RP2350` drive. The drive disappears and the board restarts into the ArduPilot bootloader, which waits for firmware on a USB serial port. On Linux the port is `/dev/serial/by-id/usb-ArduPilot_<board>-BL_<serial>-if00`.

With [picotool](https://github.com/raspberrypi/pico-sdk-tools/releases), `picotool load -v -x RPI_UAVFC_bl.uf2 -f` does the same.

Loading any other UF2 through BOOTSEL, such as Betaflight or a Pico SDK program, replaces the ArduPilot bootloader. To go back to Betaflight, load its UF2 the same way; to come back to ArduPilot, repeat these steps.

### 3. Install ArduPilot

With `uploader.py`, which comes with each release (it is `Tools/scripts/uploader.py` in this repository) and needs Python 3 and pyserial (`python3 -m pip install pyserial`):

```bash
python3 uploader.py RPI_UAVFC_arducopter.apj
```

On Linux it finds the board by itself; on Windows or macOS add `--port COM5` or `--port /dev/cu.usbmodem...`. The RP2350's flash is slower to erase and check than an STM32's, and this `uploader.py` waits long enough for it.

A ground station can load the same `.apj`: in Mission Planner use Install Firmware, Load custom firmware; in QGroundControl use Vehicle Setup, Firmware, Advanced settings, Custom firmware file. If the ground station gives up with a timeout while erasing or verifying, use `uploader.py`.

On a Pico 2 or Laurel you can instead enter BOOTSEL again and copy the firmware's `.uf2` to the drive.

The board then starts ArduPilot and appears as a USB serial port (`usb-ArduPilot_<board>_<serial>-if00` on Linux) carrying MAVLink. Connect your ground station to it and configure the vehicle as for any other ArduPilot board, starting from the board's README. On a cold power-up the bootloader waits briefly for an upload before starting ArduPilot: 3 seconds on RPI_UAVFC and Laurel, 15 seconds on Pico2.

### Updating

Load the new `.apj` with `uploader.py` or a ground station, as in step 3. ArduPilot restarts into its bootloader by itself, and your parameters are kept. Save a copy of your parameters from the ground station before an update all the same.

### Recovering a board

If the board stops booting, enter BOOTSEL and start again from step 2. If a stored parameter is what stops it booting, clearing the parameters without touching the rest needs an SWD probe; that, and flashing and debugging over SWD in general, is in [the developer notes](libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md#recovering-a-board).

## Building from Source

The build runs on Linux, on Windows under WSL2, and on macOS. The steps are ArduPilot's usual ones, documented in [BUILD.md](BUILD.md) and on the [ArduPilot developer wiki](https://ardupilot.org/dev/docs/building-the-code.html).

### Get the code and the tools

```bash
git clone --recurse-submodules -b ArduPilot-4.8-rp2350 https://github.com/fossuav/ardupilot-rp2350.git
cd ardupilot-rp2350
Tools/environment_install/install-prereqs-ubuntu.sh -y    # install-prereqs-mac.sh on macOS
. ~/.profile                                              # or log out and back in
```

The installer fetches the ArduPilot ARM toolchain (gcc 10.2.1), which is what the release firmware is built with. If you cloned without `--recurse-submodules`, run `git submodule update --init --recursive` before building: the RP2350 support in ChibiOS is in the `modules/ChibiOS` submodule.

You need picotool and pioasm only to edit the RP2350's PIO programs or to load a bootloader with `./waf bootloader --upload`. Answer yes to the installer's RP2350 question (or run it with `DO_AP_RP2350_ENV=1`), or run `Tools/scripts/rp2350_pioasm.py --install`.

### Build

```bash
./waf configure --board RPI_UAVFC      # or Pico2, Laurel
./waf copter
```

The outputs are in `build/<board>/bin/`:

| File | Contents |
|------|----------|
| `arducopter.apj` | Firmware for `uploader.py` and ground stations |
| `arducopter.uf2` | Firmware for the BOOTSEL drive |
| `arducopter.bin` | Firmware image, written at the board's application address over SWD |
| `arducopter_with_bl.hex` | Bootloader and firmware in one image; erases the parameters |
| `arducopter` | ELF, with symbols |

`./waf copter --upload` builds and then loads the firmware through the ArduPilot bootloader with `uploader.py`. Under WSL2 the board's USB port is not visible to Linux until you attach it with [usbipd-win](https://learn.microsoft.com/windows/wsl/connect-usb); otherwise copy the `.apj` to Windows and load it with a ground station.

The bootloaders are prebuilt in `Tools/bootloaders/`, and the firmware build embeds them. To rebuild one, run `Tools/scripts/build_bootloaders.py RPI_UAVFC`.

`hwdef.dat` is read only at configure time, so re-run `./waf configure --board <board>` after editing it. Debug builds, SWD, profiling and how the port works are in the [RP2350 developer notes](libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md).

## What Is Included

The branch is ArduPilot master as of 2026-10-24 with the RP2350 port and the fixes below on top. All but one are open pull requests to ArduPilot. Releases are built by `Tools/RP2350/build_release.sh`; [Tools/RP2350/README.md](Tools/RP2350/README.md) describes how the branch and its releases are kept.

### The port

- RP2350 Port ([#32995](https://github.com/ArduPilot/ardupilot/pull/32995)): ChibiOS SMP across both cores with the rate loop on core 1, hardware and PIO UARTs, PWM and PIO DShot with bidirectional DShot, the PIO analog OSD (`OSD_TYPE` 6), parameter storage in flash, the bootloader, and the Pico2, Laurel and RPI_UAVFC boards. It needs the RP2350 ChibiOS port ([ChibiOS#113](https://github.com/ArduPilot/ChibiOS/pull/113)), which the `modules/ChibiOS` submodule carries.
- Per-core load measurement that keeps running while `@SYS/threads.txt` is read - local to this branch, a fix to code the port adds, to be folded into it

### microSD and logging

- Honour AP_FATFS_MAX_IO_SIZE on the microSD SPI Path ([#34476](https://github.com/ArduPilot/ardupilot/pull/34476))
- Write microSD Blocks in One SPI Exchange ([#34477](https://github.com/ArduPilot/ardupilot/pull/34477)), with its ChibiOS half ([ChibiOS#115](https://github.com/ArduPilot/ChibiOS/pull/115))
- Recover microSD Parameter Storage after an I/O Error ([#34478](https://github.com/ArduPilot/ardupilot/pull/34478))
- Mark start_new_log() IO Steps for the IO Timer Watchdog ([#34473](https://github.com/ArduPilot/ardupilot/pull/34473))
- Clear Stale Bytes in @SYS Directory Names ([#33056](https://github.com/ArduPilot/ardupilot/pull/33056))

### RC input

- SBUS Framing Anchored on the Header, with Resync on Failure ([#33057](https://github.com/ArduPilot/ardupilot/pull/33057))
- Guard CRSF against Early Startup ([#33058](https://github.com/ArduPilot/ardupilot/pull/33058))

### Sensors and rate loop

- DPS280/310/368 Poll Rate to Match the 32 Hz Sensor ODR ([#33055](https://github.com/ArduPilot/ardupilot/pull/33055))
- ICM-56686 Gyro UI Low-pass for Fast Sampling ([#34603](https://github.com/ArduPilot/ardupilot/pull/34603))
- Honour the Harmonic Notch Loop Rate Option in the Rate Thread ([#34436](https://github.com/ArduPilot/ardupilot/pull/34436))

### Ground station and OSD

- Wake the MAVLink FTP Worker When a Request Is Queued ([#34474](https://github.com/ArduPilot/ardupilot/pull/34474)) - the branch carries the first version; the PR has since added a timeout that closes a session whose reply cannot be sent
- No Hole in the OSD Backend List ([#34497](https://github.com/ArduPilot/ardupilot/pull/34497))
- Count Unallocated Core Memory in @SYS/memory.txt ([#34530](https://github.com/ArduPilot/ardupilot/pull/34530))

## Known Limitations

These are the ones that matter when flying. The full list of open work is in the [developer notes](libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md#known-issues-and-open-work).

- The RP2350 runs at 225 MHz, 1.5 times its 150 MHz datasheet rating. This has been validated only on the bring-up boards.
- If the board has been powered for more than about an hour, reboot it before you fly. The 71.6 minute wrap of the microsecond clock has not yet been soak-tested through arming and disarming.
- After flashing over SWD, power-cycle the board: the second core sometimes fails to start after a debugger has been attached.
- DShot600 is the only DShot rate. BLHeli passthrough, CAN and serial ESC telemetry are not available on these boards; use bidirectional DShot for RPM.
- On RPI_UAVFC the battery current input does not track load reliably, though voltage is accurate. Do not rely on current, consumed capacity or sag compensation, and leave `BATT_FS_VOLTSRC` at 0.
- On RPI_UAVFC, `ESC_CALIBRATION` 2 or 3 makes the board reboot repeatedly. Calibrate ESCs with the throttle-high procedure.
- The microSD write rate is limited by the RP2350, so high-rate logging drops messages, more so in position modes. Use the `ISBH`/`ISBD` batch sampler for spectral analysis and check `DSF.Dp` before trusting a logged rate.
- Occasionally a log file is listed with a garbled name, such as `0000012B.IN` for `00000012.BIN`. The cause is not yet known.
- RPI_UAVFC has no compass; attach one to the GPS connector's I2C bus.
- On RPI_UAVFC the analog OSD and the LED strip share a PIO block, so the LED strip does not work with `OSD_TYPE` 6.
- SBUS has not been tried with a receiver.

## Reporting Problems

Report problems with this firmware as [issues on this repository](https://github.com/fossuav/ardupilot-rp2350/issues), not on ArduPilot's tracker or forum, since the code is not yet part of ArduPilot. Say which board and release you are using (the ground station shows the firmware version and git hash at boot), and attach the `.bin` log from the microSD card.

## Developer Information

- Github repository: <https://github.com/fossuav/ardupilot-rp2350>
- RP2350 developer notes: [libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md](libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md)
- Branch and release maintenance: [Tools/RP2350/README.md](Tools/RP2350/README.md)
- ArduPilot developer wiki: <https://ardupilot.org/dev/>

## License

The ArduPilot project is licensed under the GNU General Public
License, version 3 or (at your option) any later version.

- [Overview of license](https://ardupilot.org/dev/docs/license-gplv3.html)

- [Full Text](https://github.com/ArduPilot/ardupilot/blob/master/COPYING.txt)

Raspberry Pi is a trademark of Raspberry Pi Ltd.
