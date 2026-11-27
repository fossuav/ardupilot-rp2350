# RP2350 release history

Dated record of this repository's setup, releases and refreshes, newest
first, with the work still open at the top. How the branch is built and
how a release is cut are in README.md. What is known about the port itself
is in `libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md` and in
`../ardupilot-pr-analysis/32995/README.md`.

## Open work

- RPI_UAVFC's servo outputs 5-10 and the buzzer's hand-over of its PWM
  slice have run only on the bench (2026-11-26 below). The release notes
  for the next alpha must say the LED output moved from 5 to 8: a board
  that stored `SERVO5_FUNCTION` 120 keeps it, and GP1 then carries the
  LED signal until it is set to 0. The flight wing's powered bench found
  its ESC lead on the LED connector (2026-11-27, later).
- OneShot on RP2350, output 8 at function 0 leaving the buzzer on, and
  #35410 have run only on the bench (2026-11-27 below). The flight wing
  carries a OneShot build with `ONESHOT_MASK` on its elevons, so its next
  flight is their first.
- Fly Copter on this branch before an alpha2 release: its rate thread now
  runs from AP_Vehicle (#35395, 2026-11-26 below), which has run only in
  SITL. Check `RTDT` and the "Rate CPU" message in the first log, and
  #35358's notch on the same flight: its attenuation, from the batch
  sampler's spectra before and after the filters, has not been looked at
  in the air.
- QuadPlane's rate thread stalls 3-4 ms within 0.15 s after disarm, before
  and after #35395's review (bench note 2026-11-26). Disarmed, so harmless
  as it stands; not investigated, and not looked for on Copter.
- #32995 and #35395 both change the rate thread: #32995 its stack size
  and overrun test in Copter's copy, #35395 where it lives. Whichever
  merges second has to carry #32995's two changes into
  `libraries/AP_Vehicle/rate_thread.cpp`, as this branch does; #34436
  likewise for its notch update rate.
- Open the dropped-sample print fix as its own upstream PR.
- The ChibiOS pin `f92aa3d2bd` is held on fossuav/ChibiOS by its
  `rp2350-alpha2` branch, so clones and CI fetch it, but
  `build_release.sh` wants it at `rp2350-releases`' head or a tag. It does
  not descend from `7e50cd016f`, so when alpha2 is released, tag
  `7e50cd016f` there as `rp2350-alpha1` before moving `rp2350-releases` to
  the new pin.
- The 200 MHz build has flown three times, on 2026-11-07 (below). Watch
  `DSF.Dp` in the next logs: log 8 dropped 0.3 s of logging on the ground
  after disarm, which no log on `1f8685e9` did; log 9 did not.
- The GPS failsafe and #32995's commit 48 remain unflown; the quad has no
  half-duplex port, so commit 48 needs another device or airframe.
- The quad's settings for Auto: `WP_ACC_Z` 10 with `WP_SPD_DN` 10 make
  RTL's landing come in fast, and GPS height (`EK3_SRC1_POSZ` 3) wandered
  3 m on the ground against the baro's 0.3 m. Fly RTL's landing again with
  `LAND_SPD_HIGH_MS` set or the vertical defaults, and on baro height
  unless there is a reason for GPS. The PIO OSD on its fractional
  divider has not run in flight: the quad uses MSP DisplayPort
  (`OSD_TYPE` 5), and the PIO OSD (`OSD_TYPE` 6) needs a board with
  analog video.
- Drop the two local load measurement commits, `265f165bd6` and
  `96def44796`, when ArduPilot/ardupilot#33366 merges; they are not for
  #32995. They fix an upstream bug: master's `sysStopLoadMeasure()` only
  requests a stop, taken at the next idle entry, so the
  `sysStartLoadMeasure()` straight after it in `Util::thread_info()` does
  nothing and every other `threads.txt` read freezes LOAD and PEAK, on
  STM32 as well. #33366's `ca78096f50` makes the same fix for one core;
  the local commits are its per-core form, which #32995's per-core
  measurement needs. They also change STM32 output, so they could not go
  into #32995's first 51 commits.
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
- The quad's compass: its coupling to battery current fell by about a
  third between 2026-10-30 and 2026-10-31, so the refitted `COMPASS_MOT`
  now over-compensates. Find what changed on the airframe; once it is
  fixed, fly a calibration flight with `COMPASS_MOTCT` 0 and refit.
  Charger figures for the packs are still owed to anchor the current
  scale. On 2026-11-07 the coupling was back within 10% of the refit's,
  so which pack flies, or how it is strapped, is the likelier variable;
  the compass's static offset also moved about 100 mG between logs 8 and
  9, when the pack was changed.
- With `COMPASS_MOTCT` 2 on an R3 board, the current input's drifting
  zero is applied to the compass on the ground: on 2026-11-07 it turned
  the heading -8.5 degrees cold and +20 warm, and failed the mag-field
  arming check. Throttle-based compensation (`COMPASS_MOTCT` 1) fitted the
  in-flight interference as well and is zero on the ground; not flown.
- The OSD thread has 104 of 1504 bytes of stack free in every log
  (`STAK`), with MSP DisplayPort in use. Not investigated.
- Upstream, not the port: Copter's pre-failsafe yaw reset is served from
  the GSF and fails the compass for the rest of the flight. On 2026-10-30
  it fired mid-flip as the GSF lost track and left the yaw 110 degrees
  wrong; on 2026-10-31 the GSF was 16-35 degrees wrong in Loiter while
  reporting 0.4-1.4 degrees of spread. Not raised upstream.
- Log 7 of 2026-10-31 had a radio failsafe on the ground, 37 s after
  disarming. Not investigated.

## 2026-11-27, later - alpha2 on #32995 at 90fb974382: the buzzer pads

The flight wing, flashed with alpha2 `6d5570e1d2` and powered on the
bench, showed four faults, none of them in the firmware. The ESC's signal
lead was on the LED connector, so bidirectional DShot on output 1 never
had a reply: the PIO state machine restarted about 510 times a second and
sent a frame every second update. The GPS, one elevon servo and the
battery reading were put right on the bench as well. The buzzer was
silent, with no ", buzzer off": it is a module with its own on/off
switch, and the switch was off.

Before that turned up, the Copter test quad showed GPIO3 toggling at
4 kHz with 50% duty during a tune and idle otherwise, so the firmware
drives it. The schematic showed that the board has no buzzer of its own:
GPIO3 switches a MOSFET on the low side of the BZ pad, and a buzzer goes
across the BZ and 3V3 pads, + to 3V3. The hwdef said a transducer was
fitted, taken from another design that fits one; #32995's `90fb974382`
corrects the hwdef, the board README and the hardware notes, and the
README here lists it. The entry below said the wing's buzzer plays; that
was its PWM in the registers only, and it is corrected there.

The 76 commits above the PR replayed onto the new head without
conflicts. The old head is `6d5570e1d2`.

## 2026-11-27 - alpha2 on #32995 at 81a63460db: OneShot, with #35410

The alpha2 branch moves from #32995 at `6ce2608538` to `81a63460db`, four
commits on, and takes #35410 after #35358:

- OneShot and OneShot125 run on RP2350. Its PWM has no one-pulse mode, so
  a oneshot group's counter runs to TOP 0xFFFF and each trigger writes TOP
  into it, wrapping it on the next count. Before, OneShot was refused and
  those outputs fell back to plain PWM: Plane's `ONESHOT_MASK` servos ran
  at 50 Hz.
- On RPI_UAVFC, output 8 with function 0 is left out of its group, so the
  buzzer keeps the PWM slice they share. Before, function 0 took it and
  the RCOut banner said ", buzzer off".
- #35410 stops the fast rate loop waiting for each OneShot pulse. With
  `FSTRATE_ENABLE` and OneShot motors a 1591 Hz rate thread ran at about
  940 Hz; that is upstream behaviour, which RP2350 met once OneShot ran.

Bench only (`../ardupilot-pr-analysis/32995/bench-2026-11-27-oneshot-buzzer.md`
and `../ardupilot-pr-analysis/35410/`). The flight wing's `ONESHOT_MASK`
elevons got 1500 us pulses at the loop rate, and with output 8 at
function 0 its buzzer's PWM ran at 4 kHz, seen in the registers only
(2026-11-27, later). On the Copter test quad the rate thread held
1591 Hz with OneShot motors, a dt median of 674 us against 1062 us, and
the motors got 896 pulses a second against 921 before. The flight wing
runs a OneShot build from before this refresh, local `a600ccc8d7`, with
`SCHED_LOOP_RATE` 50.

The 72 commits above the PR replayed onto the new head without conflicts,
#35410's three after #35358's, and the tree differs from the old alpha2
only in the five files the new commits change. The old head is local
branch `pre-rebase/ArduPilot-4.8-rp2350-alpha2-20261127`.

## 2026-11-26 - alpha2 on #32995 at 6ce2608538: RPI_UAVFC outputs 5-10

The alpha2 branch moves from #32995 at `b9db33af08` to its head
`6ce2608538`, 18 commits on. Six are the PR's own: Rover's hot code
placed in SRAM, the placement registry scripts, and waf's
`--rp2350-relocate`. Twelve add RPI_UAVFC's servo outputs, local to this
branch until today:

- The Spare IO pads GP1-GP3 are servo outputs 5-7, and the LED connector
  moves from output 5 to output 8, with `SERVO8_FUNCTION` 120 its default.
- RP2350 supports alternative configs, with PWM outputs, and
  `BRD_ALT_CONFIG` 1 makes the ESC connector's TEL pin and the DVTX
  connector's SBUS pin outputs 9 and 10.
- Output 8 shares its PWM slice with the buzzer. Given any function but an
  LED strip, the output has the slice, the buzzer is off, and the RCOut
  banner ends ", buzzer off".
- The settings every RP2350 board shares moved out of the three hwdefs
  into `rp2350_halconf.h`, `rp2350_mcuconf.h` and `PICO2.py`, leaving
  Copter, Plane and the bootloaders byte-identical.

All of it was bench-tested on an RPI_UAVFC on 2026-11-26
(`../ardupilot-pr-analysis/32995/bench-2026-11-26-buzzer-outputs.md`):
each output's pulse read back over SWD, and the buzzer's tone measured
with the LED connector as a strip, as a servo output and as a script's
strip. The script's strip left the buzzer at 2.66 kHz instead of 4 kHz;
the buzzer commit now restarts the slice on the tone alarm's clock. None
of it has flown.

The 63 commits above the PR replayed onto the new head with two
conflicts, both where the PR's hwdef cleanup meets the local QuadPlane
loop rate: the RPI_UAVFC and Laurel hwdefs, and the developer notes'
loop-rate table. Apart from the six PR commits the tree is the one this
work had on the old alpha2. RPI_UAVFC, Laurel and Pico2 build Copter,
Plane and the bootloader.

## 2026-11-26 - the notch's sine and cosine without libm, #35358

ArduPilot/ardupilot#35358 joins alpha2 after #35395, at its PR head
`bd5a581d84`. The harmonic notch now computes the sine and cosine of each
notch it re-centres together, from Cephes' polynomials, instead of
calling libm's `sinf()` and `cosf()`, which run from flash on the RP2350.
A local registry commit places its helper, `calculate_sinf_cosf()`, in
SRAM beside its caller. On the bench on 2026-11-23, Copter with RPM
notches, it took core1 from 67.3% to 59.8% busy
(`../ardupilot-pr-analysis/32995/bench-2026-11-23.md`, sections 14 and
16). Its unit test holds every coefficient within 6e-7 of the exact
design, and Copter and Plane build with it for every board. Its
attenuation has not been checked in flight.

## 2026-11-26 - alpha2 with QuadPlane's rate thread from #35395

The alpha2 branch, on #32995 at `b9db33af08` since 2026-11-25, now
carries ArduPilot/ardupilot#35395 after the other follow-up PRs. #35395
moves Copter's rate thread into AP_Vehicle and lets QuadPlane run its
VTOL rate controller in it, through `Q_FSTRATE_ENABLE` and
`Q_FSTRATE_DIV`. The branch carries its head, `a6159c5891`, except that
the moved code keeps what this branch had already changed in Copter's:
#32995's board-set stack size (`AP_RATE_THREAD_STACK_SIZE`, 3072 on these
boards against master's 1536) and separate-core overrun test, and
#34436's notch update rate. The RP2350 SRAM registry commits sit between
#35395's where a commit needs them to build, and four local commits
follow: QuadPlane's 200 Hz main loop and rate thread defaults on RPI_UAVFC
and Laurel, their documentation, and removing the dropped-sample print
that stalled DShot. Every new commit builds Copter and Plane for
RPI_UAVFC. The branch was built as `rp2350-quadplane-rate-thread`; the
alpha2 branch before it is `dcf0858c76`.

Copter now runs its rate thread from AP_Vehicle. The code is Copter's,
moved, and #35395's SITL autotests cover it, but it has not run on Copter
hardware.

QuadPlane on the wing's bench, ArduPlane `dfba74c2`
(`../ardupilot-pr-analysis/32995/bench-2026-11-26-quadplane.md`):
#35395's review moved `calc_pwm()` from the thread to the main loop, so
the thread only sends finished outputs. Disarmed in FBWA with the stick
at 1800, the forward throttle stayed at its minimum in all 694 samples.
The thread at 1.6 kHz with a 200 Hz main loop saves 10 points of core0
against QuadPlane at 300 Hz without it, as before the review, and core1
is 10 points less busy than with the earlier version (`488cd840ff`,
benched on 2026-11-25 and kept as
`pre-refresh/rp2350-quadplane-rate-thread-20261126-0846`). No long loops
armed. The ESC calibration change has run only in SITL. QuadPlane has not
flown on the port.

## 2026-11-07 - an Auto mission on the alpha2 branch

Log 9, an hour after log 8, on a fresh pack and with `EK3_SRC1_POSZ`
changed from baro to GPS: takeoff to 25 m, ten spline waypoints and RTL,
the first Auto flight on the port. The port was as clean as in log 8,
with a worst rate-loop gap of 0.93 ms in flight and nothing dropped, and
the 13 mission items stored and read back. Every waypoint was reached,
within 0.8-4.8 m.

The quad's waypoint settings (`WP_SPD` 40, `WP_ACC` 60, `WP_ACC_CNR` 80,
`WP_JERK` 120) ask for more than its 30 degree lean limit gives, so it
flew at the limit for 94% of the mission and fell up to 17 m behind its
target on tight turns. RTL came home and began to land, but never slowed
to `LAND_SPD_MS`: `WP_ACC_Z` 10 with `WP_SPD_DN` 10 starts the slowdown too
late (in SITL it touches down at 0.8 m/s with both, 0.5 with either alone
or with `LAND_SPD_HIGH_MS` 2), and GPS height had wandered 3 m on the
ground, so home was set low. The pilot took over at about 0.7 m. Both are
configuration, not the port.

Measurements in `../ardupilot-pr-analysis/32995/bench-2026-11-07.md`,
"Log 9".

## 2026-11-07 - first flights of the alpha2 branch

Two flights in one log from Andy's quad on `4d4220d940`, this branch, with
the parameters of the last flight on `1f8685e9` but for the gyro
calibration. #32995's head has since moved to `1fcec947c5`, whose tree is
identical to `f0f78b48ec`, so the flown code is the PR's. These are the
first flights of the 200 MHz clocks, the continuous-read flash, the new
SRAM placement and bidirectional DShot's fractional PIO divider. The PIO
OSD did not run, as the quad uses MSP DisplayPort.

The port was clean in flight: rate loop 1600 Hz with a worst gap of 0.91
ms, 2.23 ms at the arming instant against 4.3-6.5 ms on `1f8685e9`, no long
loops at any time, nothing dropped on any UART, and DShot telemetry from
all four ESCs with no errors. Scheduler load fell from 54-69% in flight to
29-41%, and from 57% to 32% disarmed, about twice the fall the bench
measured. Free heap is about 6 KB less. 281 log messages, 0.3 s of every
stream, were dropped on the ground 1.4 s after the last disarm while both
loops ran normally; the card's write latency and the new 25 MHz microSD
clock are the candidates. Nothing on `1f8685e9` dropped.

RTL flew for the first time, by switch: home from 34 m at 20 m, a 3.4 m
overshoot, held within 1 m, and its descent begun before the pilot took
over at about 11 m. Its landing, Auto, the GPS failsafe and #32995's
commit 48 remain unflown.

The GPS took 13 minutes to reach an EKF origin, as on the first power-up
of 2026-10-29 and 2026-10-31. Meanwhile the board warmed to 25 degC and
the current input read up to 7 A with the motors stopped, which
`COMPASS_MOT` turned into up to 176 mG on the compass: the mag-field
arming check failed on it, and would have read half as much without.
The compass's coupling to battery current was back near 2026-10-30's.

Measurements in `../ardupilot-pr-analysis/32995/bench-2026-11-07.md`.

## 2026-11-07 - alpha2 branch on #32995 at f0f78b48ec, 200 MHz

`ArduPilot-4.8-rp2350-alpha2`, for the next release, was rebuilt on
#32995's head `f0f78b48ec`: 71 commits on master `2f3644c90e`, of which
the first 55 are the `e45dae01b1` the branch already carried. Master,
the follow-up PRs, the local commits and this repository's commits are
unchanged: 29 of the 30 replayed commits have the same patch, and
#34477's ChibiOS update differs only in its pin. Apart from the pin, the
tree changed exactly as #32995 did. The branch as it was is kept locally
as `pre-rebase/ArduPilot-4.8-rp2350-alpha2-20261107`.

The 16 new commits run all three boards at 200 MHz from a 1200 MHz VCO,
with the flash at 100 MHz in continuous read and 3 clocks between reads,
in place of 225 MHz with a 75 MHz flash clock. Bidirectional DShot and the
OSD take fractional PIO dividers to suit. More of both cores' hot paths
move to SRAM, external AHRS is compiled out (40 KB of flash, 544 bytes
more heap), `Scheduler::delay()` drops a 64-bit divide, and SPI runs at
the fastest clock not above the request, which puts the microSD at 25
MHz. On the RPI_UAVFC bench, armed with DShot, logging and the OSD,
#32995 measures the scheduler load at 36% at 225 MHz, 31% at 200 MHz,
and 25% with the core0 placement and external AHRS out. RPI_UAVFC's
flash read window was swept; Laurel and a Pico 2 read their whole flash
back correctly at 200 MHz. All of it is bench only: nothing
has flown. The bootloaders are not rebuilt, as #32995 changes no
`hwdef-bl.dat`.

They need #113's new head, `39ed91cb62`, which adds the continuous read
and deselect time, off unless a board defines them. #115 was reapplied
on it unchanged (same patch-id) as `f92aa3d2bd`, now the pin, held
locally on branch `rp2350-alpha2` in `modules/ChibiOS`.

RPI_UAVFC, Pico2 and Laurel build Copter and the bootloader with no
warnings, as CI does; RPI_UAVFC's Copter uses 1423236 bytes of flash.
Not yet run on a board.

## 2026-11-02 - rp2350-alpha1

ArduCopter 4.8.0-dev for RPI_UAVFC and Pico2, tagged at `5548cfd4c3` and
published 2026-11-02 10:35 UTC as "ArduCopter 4.8.0-dev for RP2350 alpha1
(2026-11-02)", with the same 16 files as `rp2350-20261027`. "alpha1" is
the release's name only: the firmware reports `ArduCopter V4.8.0-dev
(324af528)`, the commit's ID when it was built, before the branch's
history was rewritten on 2026-11-04. It is not marked pre-release, so
`releases/latest` serves it. As before, it was created as a draft with
`gh` and published from the GitHub page.

`5548cfd4c3` differs from `1f8685e9`, the build flown on 2026-10-29, 30
and 31 (eleven flights in Stabilize, Alt Hold, Loiter and Acro), only in
files the firmware is not built from, so the release is the flown code.
RTL, Auto, the GPS failsafe and #32995's commit 48 remain unflown.

Since `rp2350-20261027` the firmware has changed only by #32995 moving
from `1b8b7e68cf` to `e45dae01b1` (2026-10-28 below): the rebuilt
bootloaders with the fast erase and `GET_CRC` fixes, the half-duplex PIO
UART echo window, `@SYS/flash.bin` on the XIP window, and Pico2's
defaults. Master, the follow-up PRs and the local commits are unchanged,
and `uploader.py` is byte-identical to the last release's. The notes tell
users to install the new bootloader when upgrading, and give the current
input and compass compensation limitations from the README. They also say
the flights had no long loops in any log, which holds only in flight: log
7 of 2026-10-31 had one at the arming instant, on the ground.

The ChibiOS pin is still `7e50cd016f`, the head of fossuav/ChibiOS
`rp2350-releases` and tagged `rp2350-20261027` there, so nothing was
pushed to it. The fresh-clone check of 2026-10-27, that a `git clone
--recurse-submodules` builds byte-identical firmware, was not repeated;
`build_release.sh` reused the existing build directories.

## 2026-10-31 - release-testing flights on 1f8685e9

Two logs, three flights, from Andy's quad on `1f8685e9`, with no parameter
changed since the last flight of 2026-10-30. The port was clean again:
rate loop 1600 Hz with a worst gap in flight of 1.05 ms, the IMU reaching
the EKF every 5.00 ms, nothing dropped, no EKF failsafe or emergency yaw
reset. Log 7's one long loop, and rate-loop gaps of 5-6 ms on two of the
flights, come at the arming instant, on the ground at spin-arm: the
parameter save that arming
makes, which `AP_STORAGE_NO_WRITE_WHILE_ARMED` keeps out of flight.
2026-10-30's flights show the same at arming (4.3-6.5 ms), which its
"worst armed gap" missed by counting the flights only. Every
`SERIALn_OPTIONS` is still 0, so commit 48 is still unflown.

The compass was no longer compensated: in Acro it read 40 degrees and
more off the EKF's heading at high throttle, against 6-10 the day before.
Its coupling to battery current fell by about a third overnight while the
current input's scale stayed the same, so 40-48% of what the refitted
`COMPASS_MOT` applies is now excess. What changed on the airframe is not
known. The EKF yaw stayed within about 6 degrees of true throughout,
checked against GPS acceleration.

Measurements in `../ardupilot-pr-analysis/32995/bench-2026-10-31.md`.

## 2026-10-30 - release-testing flights on 1f8685e9

Four logs from Andy's quad on `1f8685e9`, the tree this branch still has.
The port was clean on every flight: rate loop 1600 Hz with a worst gap in
flight of 1.07 ms, no long loops, the IMU reaching the EKF every 5.00 ms,
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
