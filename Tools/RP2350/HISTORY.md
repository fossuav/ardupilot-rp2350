# RP2350 release history

Dated record of this repository's setup, releases and refreshes, newest
first, with the work still open at the top. How the branch is built and
how a release is cut are in README.md. What is known about the port itself
is in `libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md` and in
`../ardupilot-pr-analysis/32995/README.md`.

## Open work

- Drop the two local load measurement commits, `1eebac56d2` and
  `c03191be64`, when ArduPilot/ardupilot#33366 merges; they are not for
  #32995. They fix an upstream bug: master's `sysStopLoadMeasure()` only
  requests a stop, taken at the next idle entry, so the
  `sysStartLoadMeasure()` straight after it in `Util::thread_info()` does
  nothing and every other `threads.txt` read freezes LOAD and PEAK, on
  STM32 as well. #33366's `ca78096f50` makes the same fix for one core;
  the local commits are its per-core form, which #32995's per-core
  measurement needs. They also change STM32 output, so they could not go
  into #32995's first 51 commits.
- A release with the rebuilt bootloaders. `rp2350-20261027` ships the ones
  from before #32995's fast erase and `GET_CRC` fixes of 2026-10-28.
- Take #34474's current head. The branch carries its first version, without
  the timeout that closes an FTP session whose reply cannot be sent.
- Untested: loading an `.apj` from Mission Planner or QGroundControl. This
  repository's `uploader.py` allows for the RP2350's slow erase and CRC; a
  ground station's own uploader may time out.
- Laurel binaries, once Laurel has flown on the current code.
- A single UF2 holding the bootloader and the firmware, skipping the
  parameter area, would make a first install one copy. Needs trying on a
  board.
- Raspberry Pi's trademark rules want written consent for the logo outside
  product listings. The README carries it; no consent is recorded here.
- The `main` branch still holds the placeholder README from the
  repository's creation.
- `grant_push.py` grants only branches of this repository, so a push from
  `modules/ChibiOS`, as for `rp2350-releases`, has to be run by hand. The
  submodule has no pre-push hook.
- A Renode test in this repository's CI, to add when Renode support for
  the RP2350 is incorporated.
- R4 boards: measure the current input's disarmed zero at a cold boot and
  warm, repeat a calibration flight and refit the quad's compass. Then
  update the port's current-input text (`rp2350/DEVELOPMENT.md`, the
  RPI_UAVFC README and `defaults.parm`'s comment), which describes only
  the 82.5k R3 boards and now disagrees with the top-level README.
- Until then, the quad's current-based compass compensation holds only
  near the 11-19 degC board temperature it was fitted at; recheck it on a
  warmer day.
- The next release's notes: the current input and compass compensation
  limitations the README now gives.
- The OSD thread has 104 of 1504 bytes of stack free in every log
  (`STAK`), with MSP DisplayPort in use. Not investigated.
- Upstream, not the port: Copter's pre-failsafe yaw reset is served from
  the GSF and fails the compass for the rest of the flight. On 2026-10-30
  it fired mid-flip as the GSF lost track and left the yaw 110 degrees
  wrong. Not raised upstream.

## 2026-10-30 - release-testing flights on 1f8685e9

Four logs from Andy's quad on `1f8685e9`, the tree this branch still has.
The port was clean on every flight: rate loop 1600 Hz with a worst armed
gap of 1.07 ms, no long loops, the IMU reaching the EKF every 5.00 ms,
nothing dropped. No port runs half duplex, so #32995's commit 48 is still
unflown.

Flight 2 had three EKF failsafes in Acro and a lean. The compass caused
them. Motor current left up to 480 mG of interference against a 150 mG
horizontal field, because the current input read 5.8 times low and the old
`COMPASS_MOT` was too small and misdirected besides. A pre-failsafe yaw
reset then took the GSF's yaw mid-flip and locked the compass out.

On this quad (a modified R3 and an iFlight BLITZ E55) the current input
tracks load at 3.2-3.7 mV/A against the eRPM current model, on a zero
that drifts 1.9-2.9 mV/degC with board temperature. The model read about
30% low against the charger on 2026-10-29, so the true gain may be nearer
2.4 mV/A; charger figures for these packs would settle it. With `BATT_AMP_PERVLT` 295, `BATT_AMP_OFFSET` 0.035 and
the compass refitted from a calibration flight, Acro flew clean: no
failsafe or yaw reset, and the compass within 7-9 degrees of the EKF's
heading at every throttle, against 48-68 before. The top-level README's
Known Limitations now give the current input's gain and drift, and their
effect on compass motor compensation.

Measurements in `../ardupilot-pr-analysis/32995/bench-2026-10-30.md`.

## 2026-10-29 - first flights on e45dae01b1, this repository's commits squashed

`1f8685e9a7` flew twice on Andy's quad (RPI_UAVFC, a modified R3), the
first flights of #32995 at `e45dae01b1`: rate loop 1600 Hz, worst gap 0.97
ms, no log drops. They do not clear #32995's two commits on top: no port
on the quad runs half duplex, so 48's echo window never ran, and 49's
receive FIFO bound cannot be reached by a working state machine. The
GPS's failure to fix in those flights was the receiver recovering, not
the firmware. Measurements are in
`../ardupilot-pr-analysis/32995/bench-2026-10-29.md`.

The planned fold of the two local load measurement commits into #32995
was dropped, as Open work explains.

This repository's own 18 commits, the top-level README and its photo,
this directory and the build workflow, were squashed into three, with the
tree unchanged.

## 2026-10-28 - #32995 at e45dae01b1, with the PIO UART echo fix

Last, #32995 moved to `e45dae01b1`, 55 commits on master `2f3644c90e`, and
this branch was rebuilt on it. The bootloader's fast erase now skips a 64
KB block only when its first and last 4 KB pages both read erased, which
catches the RP2350-E10 UF2 workaround page at the top of flash (on a Pico
2 it survived and failed `uploader.py --verify-firmware-is`);
`uploader.py --force-erase` erases every block, and `GET_CRC` in a session
that uploaded nothing returns the readback. Laurel's bad-firmware beep is
gone, as its buzzer is not self-oscillating. All three bootloader binaries
are rebuilt. Two new commits make `@SYS/flash.bin` read the RP2350's XIP
window rather than STM32's `0x08000000`. The PIO UART commits are now
`2f880ec7b4` and `854679d856`. The rebuild changed exactly what #32995
changed, the bootloader binaries included; RPI_UAVFC, Pico2 and Laurel
build with no warnings, and so does the RPI_UAVFC bootloader.

Before that, #32995 moved to `d5cb7ecb59`, still 53 commits on
master `2f3644c90e`, and this branch was rebuilt on it. The two PIO UART
commits moved unchanged to 48 and 49, as `426f25a3bb` and `afd80cc619`,
ahead of the four STM32-changing commits. The other changes are to the
boards: the `Perf:` debug report is off on Pico2 and Laurel, and Pico2
defaults to no log backend (it has no microSD, so MAVLink logging failed
pre-arm until a GCS streamed it; `LOG_BACKEND_TYPE` 2 turns it back on)
and sets accelerometer offsets so that only RC fails its pre-arm checks.
The rebuild changed exactly what #32995 changed, the same five files and
lines; RPI_UAVFC, Pico2 and Laurel build with no warnings.

Earlier, the branch was rebuilt on #32995's head `6cffb8ed11` with two new
commits on top, `7873a96aa0` and `9a9765e128`, and the branch's own
commits replayed after them. The two commits were then pushed to #32995
as `9a9765e128`. #32995 had taken the three doc
corrections, so the rebase dropped `342690e663`, `00f9f2579d` and
`323fb63aee` as already applied; otherwise it brought only a blank line
in `bin2uf2.py`. Master, the follow-up PRs and the ChibiOS pin are
unchanged. No release was cut.

The two commits answer the review finding on half-duplex PIO UARTs: the
receive interrupt dropped everything as our own echo until the caller
next called flush() or available(), so a reply that arrived first was
lost. The window now closes in the interrupt, on a flag the PIO transmit
program raises as it releases the line, and every receive FIFO loop
reads at most 16 entries, so no FIFO state can hang the interrupt or a
section with interrupts masked. A quick /pr-review found two cross-core
races in the first version; both were fixed before the commits were
final.

Bench, RPI_UAVFC R4 with an analog SmartAudio v2 VTX on SERIAL4 (the VID
connector): with AP_SmartAudio changed for the test to skip its flush and
read 150 ms after each request, the 35 s after boot went from 276 bytes
sent and none received to one request and its 11 byte reply. Unchanged
SmartAudio read the VTX at boot and set channel, power (25 and 200 mW)
and pit mode in every run, before and after; about one run in four
needed one retried SET, about 6 s, on either build. SERIAL4's framing
error count rises with each exchange, which fits the line resting low on
the pull-down between turnarounds; not investigated.

## 2026-10-27 - rp2350-20261027, the first release

ArduCopter 4.8.0-dev for RPI_UAVFC and Pico2, built at `955a29be43` and
published 2026-10-27 15:32 UTC: for each board the `.apj`, `.uf2`, `.bin`,
`_with_bl.hex`, ELF and bootloader `.uf2` and `.bin`, plus `uploader.py` and
`SHA256SUMS`, 16 files. It is not marked pre-release, so `releases/latest`
serves it. A fresh `git clone --recurse-submodules` of the branch built
both boards with `.bin` and `.apj` byte-identical to the release.

The auto-mode safety check refused `gh release create` for a public
release, so it was created as a draft and published from the GitHub page.

Changes since 2026-10-25, all documentation:

- Top-level README: the Raspberry Pi logo at the end of the title, loaded
  from raspberrypi.com and linked to it as their trademark rules ask, with
  the trademark acknowledgement; BOOTSEL with the BOOT button on every
  board; SBUS and the garbled log names dropped from Known Limitations; PR
  references as links, which markdownlint (MD034) wants.
- The port's docs, local until folded into #32995: the RPI_UAVFC and
  Laurel READMEs and DEVELOPMENT.md now give the BOOT button, SBUS as run
  with a receiver (by David Buzz), and the garbled log names as no longer
  seen. Which SPI change cured those was not isolated. ChibiOS `773973509`
  (2026-09-07) fits the symptom, but the 2026-09-08 measurements found its
  paths not taken; ArduPilot/ChibiOS#115 changed how directory sectors are
  written.

GitHub Actions turned out to be on, running upstream's workflows: the
release tag ran test WebAssembly Plane and test Renode, and the push of
`8ed4370cdc`, a change under `Tools/`, started 22. test Renode failed on
the branch and the tag because `git rev-parse` in its container rejected
the checkout as "dubious ownership", which is the workflow on a fork, not
the code: it passes upstream. The 21 runs still going were cancelled, and
`Tools/RP2350/disable_upstream_ci.sh` turned off all 34 upstream workflows
as a repository setting, leaving the tree's `.github` as upstream's.
`.github/workflows/rp2350_build.yml` replaces them: Copter and the
bootloader for RPI_UAVFC, Pico2 and Laurel on branch pushes and pull
requests, skipping changes to `.md` and `.jpg` files only, with one ccache
entry per board written only by the release branch.

After the release, the ChibiOS pin `7e50cd016f` (#113 plus #115) was found
to be on no ArduPilot/ChibiOS branch, held only by the head of
andyp1per/ChibiOS `rp2350-flight-test`. ArduPilot/ChibiOS was forked to
fossuav/ChibiOS (default branch only) and the pin pushed there as branch
`rp2350-releases` and tag `rp2350-20261027`. `8ed4370cdc` makes
`build_release.sh` refuse a pin that is neither. This does nothing for the
`check-merge-commits` failure on #32995, which needs #113 merged.

## 2026-10-25 - the repository set up

fossuav/ardupilot-rp2350 was created on 2026-10-25 with `main` (a
placeholder README), `master` (upstream) and `ArduPilot-4.8-rp2350`, then
equal to `rp2350-flight-test` at `cfc250c56a`:

- ArduPilot master at `2f3644c90e`;
- the 51 commits of #32995 at `1b8b7e68cf`;
- the follow-up PRs #33055, #33056, #33057, #33058, #34436, #34473, #34474
  (first version), #34476, #34477, #34478, #34497, #34530 and #34603;
- the two local load measurement commits.

Branch commits were matched to the PR heads by patch-id, and by content
where a commit had been reworded or, for #34477, its ChibiOS bump differed.

Added the top-level README, modelled on smallfastdrone's, and this
directory's README and `build_release.sh`. On 2026-10-26 the default branch
moved from `main` to `ArduPilot-4.8-rp2350`. Issues are enabled.

Decisions:

- Binaries go on GitHub releases rather than into the tree, under names
  with no version, so `releases/latest/download/<file>` links stay valid.
- Copter only, the only vehicle flown on RP2350, for RPI_UAVFC (the board
  that flies) and Pico2 (Simulation on Hardware on a stock Pico 2). Laurel
  is build-from-source.
- `uploader.py` ships with each release: it carries the RP2350 erase and
  CRC timeouts and needs only pyserial.
- smallfastdrone's filter keeping CI off tag pushes was not carried,
  because no workflow runs had been listed and Actions was taken to be
  off. That was wrong: Actions is on and runs upstream's workflows. The
  first push of `master` ran the suite; branch pushes run what the path
  filters let through, about 22 workflows for a change under `Tools/`; and
  the release tag ran test WebAssembly Plane and test Renode.

Found while building: a clone made without submodules fails
`./waf configure` on `modules/lwip` until `git submodule update --init
--recursive` is run, so the README leads with `--recurse-submodules`.
