# RP2350 alpha2 flight card: performance and RAM profile

Three flights on the RPI_UAVFC quad (BLITZ E55, M10 GPS, ELRS, MSP
DisplayPort OSD, four NeoPixels). They answer:

1. How busy each core is in real flight, measured cleanly.
2. What LED_roll costs in flight, on and off in the same log.
3. Whether the right code is in SRAM.

Fly them in order. Stop and look at the data if a flight goes wrong.

## Build

```bash
git fetch origin
git checkout -B ArduPilot-4.8-rp2350-alpha2 origin/ArduPilot-4.8-rp2350-alpha2
git submodule update --init --recursive
git rev-parse --short=8 HEAD
mkdir -p ~/rp2350-flight/flight ~/rp2350-flight/profile

./waf configure --board RPI_UAVFC
./waf copter
cp build/RPI_UAVFC/bin/arducopter.apj ~/rp2350-flight/flight/
cp build/RPI_UAVFC/bin/arducopter ~/rp2350-flight/flight/arducopter.elf

./waf configure --board RPI_UAVFC \
    --extra-hwdef=Tools/RP2350/flight-test/flight-profile.hwdef
./waf copter
cp build/RPI_UAVFC/bin/arducopter.apj ~/rp2350-flight/profile/
cp build/RPI_UAVFC/bin/arducopter ~/rp2350-flight/profile/arducopter.elf
```

`checkout -B` discards unpushed work on that laptop's copy of the branch.
Both images report the same hash, so label every log by flight number.
Keep each `.elf`: the profile can only be read with the one that flew.

Build hash: `________`

Flash with `Tools/scripts/uploader.py --port <port> <file>.apj`. Loading
an `.apj` from a ground station is untested on this port.

## Known hazards

- First flights on this base: Picolibc, reworked DShot and UART locking.
  The bench board has no motors, so DShot into real ESCs is unproven until
  the props-off check below.
- Reboot before every flight, and do not fly on a boot older than 60
  minutes. Arming across the 71.6 minute `micros()` wrap is unproven.
- No RTL or Auto on this card. On 2026-11-07 RTL's landing came in too
  fast and the pilot took over at 0.7 m. Land by hand.
- With `COMPASS_MOTCT` 2 the current input drifts as the board warms. On
  2026-11-07 that moved the ground heading -8.5 to +20 degrees and failed
  the mag-field arming check. Avoid long waits powered up. Before Loiter,
  check the heading matches the nose; if not, fly Alt Hold instead.
- The GPS has taken up to 13 minutes to give an EKF origin. Do not wait it
  out for this card: the hover leg can be flown in Alt Hold.
- "Takeoff blocked: CPU overload" can be a start-up transient of the load
  measurement. Disarm and reboot. If it repeats, set `BRD_IDLE_STATS` 0 for
  that flight and note it, as `RPLD` load is then not valid.

## Settings

| | Flight 1 | Flight 2 | Flight 3 |
|---|---|---|---|
| Purpose | baseline | LED_roll | RAM profile |
| Firmware | `flight/` | `flight/` | `profile/` |
| `SCR_ENABLE` | 0 | 1 | 0 |
| `SCR_DEBUG_OPTS` | 0 | 8 | 0 |
| `SERVO5_FUNCTION` | as found | 94 | as found |
| `APM/scripts` | - | `LED_roll_cycle.lua` only | - |

On every flight: `BRD_IDLE_STATS` 1, `LOG_BITMASK` 442366, `OSD_TYPE` 5.
Leave `LOG_FILE_BUFSIZE` unset: a stored value cannot be unset over
MAVLink. Reboot after changing any of these.

`SERVO5_FUNCTION` as found: `________`

## Props-off check, after the first flash

- [ ] Log reports the build hash above
- [ ] Motor test: each motor in order, correct direction
- [ ] ESC telemetry shows RPM on all four motors, no errors
- [ ] RC, modes and e-stop switch respond correctly
- [ ] Arm for 30 s, disarm, pull the log
- [ ] `RPLD` about once a second, `C0` and `C1` non-zero after arming
- [ ] `PM.NLon` 0 while armed, `DSF.Dp` 0

## Flight profile

Fly the same profile each time, on a similar pack.

1. Take off in Stabilize or Alt Hold and hover at about 2 m for 30 s.
   On flight 1, land and inspect if anything looks or sounds wrong.
2. Hover in Loiter for 2 minutes (Alt Hold if no origin or heading is off).
3. Fly hard for 2 minutes in Acro or Stabilize: punch-outs, fast forward
   flight and turns.
4. Land by hand and disarm.

**Land now** on oscillation, a motor or ESC fault, an unexpected mode
change, any failsafe, an "Internal Error" message, or RC or video dropouts.

## Flight 1: baseline

- [ ] Settings checked, rebooted
- [ ] Flown, log pulled

Log: `____` Pack: `____` Start/end V: `____ / ____`

## Flight 2: LED_roll

Copy `Tools/RP2350/flight-test/LED_roll_cycle.lua` to `APM/scripts`, with
no other LED script there.

- [ ] Settings checked, rebooted
- [ ] Strip shows colour for 15 s, then dark for 15 s, on the ground
- [ ] Flown, log pulled

Log: `____` Pack: `____` Start/end V: `____ / ____`

## Flight 3: RAM profile

Flash `profile/`, set `SCR_ENABLE` 0, `SCR_DEBUG_OPTS` 0 and
`SERVO5_FUNCTION` as found, and reboot. The sampler records only while
armed, so ground time does not dilute it.

- [ ] Settings checked, rebooted
- [ ] Flown, landed, **disarmed, battery left connected**
- [ ] USB plugged in with the battery still on
- [ ] `ftp get @SYS/pcprof0.txt pcprof0.txt` (core0)
- [ ] `ftp get @SYS/pcprof.txt pcprof.txt` (core1)
- [ ] First line of each reads `PROF c<n> n=... drop=... idle=...`
- [ ] Log pulled, only then power off

The counts are lost on any reboot or power-off. The firmware refuses the
dump while armed, as producing it stalls the main loop for over 100 ms.

Log: `____` Pack: `____` Start/end V: `____ / ____`

## What to compare

| Measure | Log field | Reference | Concern if |
|---|---|---|---|
| Core load | `RPLD.C0`, `C1` | bench 39-45%, 44-49% | core1 near saturation |
| LED_roll cost | `RPLD` by `LEDC.On` | core0 +6 points, core1 flat | core1 rises |
| Rate loop | `RTDT.dtMax` | 0.91-0.93 ms (2026-11-07) | worse with script on |
| Main loop | `PM.NLon`, `MaxT` | none, up to 5.33 ms | any long loops |
| Logging | `DSF.Dp`, `FMn` | no drops in flight | any drop |
| Free memory | `PM.Mem` | 117 KB with script (bench) | falling in flight |
| Script stack | `STAK` | 3 KB of 10 KB (bench) | nearing 10 KB |
| XIP cache | `RPLD.XHit`, `XAcc` | 88-90%, 8-10 M/s (bench) | hit rate falls |

- `RPLD` load reads 3-4 points below the PC sampler. Use it to compare
  flights and windows, not as absolute CPU load. Core1's value moves in
  steps of about a point. It reads zero until the first arming.
- Scripting cuts the log buffer from 80 to 64 KB, so compare peak buffer
  use between flights 1 and 2 rather than free space.
- Skip the first 3 s of each `LEDC` window: the load average lags.
- Do not use flight 3's load figures: the sampler adds to them.
- On the bench the profile listed 81% of core0's busy samples and 96% of
  core1's. A function missing from it is not proof that it is cold. A/B
  any candidate on the bench before changing a placement registry.

## Afterwards

- Keep together: three logs, `pcprof0.txt`, `pcprof.txt`, both `.elf`
  files, and this card's notes.
- Restore the quad: `SCR_ENABLE` 0, `SCR_DEBUG_OPTS` 0, `SERVO5_FUNCTION`
  as found, remove `LED_roll_cycle.lua`.
- Drop the flight-test commits from the branch before a release.
