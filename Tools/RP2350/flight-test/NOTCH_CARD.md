# RP2350 notch update flight card

One flight on the RPI_UAVFC quad (BLITZ E55, M10 GPS with an IST8310
compass, ELRS, MSP DisplayPort OSD) to measure branch `notch-fast-sincos`.
That branch replaces the notch update's `sinf()` and `cosf()` with an
inlined polynomial. The flight answers:

1. How much core1 load it saves.
2. Whether the notch still attenuates as it did.

Fly it with the motors loaded. With motors stopped every notch centre sits
below 400 Hz, where the old code never took its expensive path, so a static
bench cannot show the saving.

## Build

```bash
git fetch origin
git checkout -B notch-fast-sincos origin/notch-fast-sincos
git submodule update --init --recursive
git rev-parse --short=8 HEAD
mkdir -p ~/rp2350-flight/notch

./waf configure --board RPI_UAVFC
./waf copter
cp build/RPI_UAVFC/bin/arducopter.apj ~/rp2350-flight/notch/
cp build/RPI_UAVFC/bin/arducopter ~/rp2350-flight/notch/arducopter.elf
```

`checkout -B` discards unpushed work on that computer's copy of the branch.
Do not add `flight-profile.hwdef`: this flight's load figures need a build
without the PC sampler.

Build hash: `________`

Flash with `Tools/scripts/uploader.py --port <port> <file>.apj`, with
Mission Planner and anything else on that port closed.

## Known hazards

- First flight of this filter code. It has run only in host tests and SITL.
  A wrong coefficient would show as oscillation or a hot motor.
- Reboot before flying, and do not fly on a boot older than 60 minutes.
- No RTL or Auto. Land by hand.
- "GPS Glitch or Compass error" is expected in Acro: log 15 had seven with
  no failsafe. Do not switch to a position mode while it is up.

## Settings

Leave the quad as it flew log 15, and note anything that differs:

`BRD_IDLE_STATS` 1, `LOG_BITMASK` 442366, `OSD_TYPE` 5, `SCR_ENABLE` 0,
`SERVO5_FUNCTION` 120, `INS_HNTCH_OPTS` 22, `INS_HNTCH_HMNCS` 15,
`INS_LOG_BAT_MASK` 3, `INS_LOG_BAT_OPT` 4, `COMPASS_MOTCT` 2, and Relay2
(RC12) high so that the VTX answers.

## Props-off check

- [ ] Boot message reports the build hash above
- [ ] Motor test: each motor in order, correct direction
- [ ] ESC telemetry shows RPM on all four motors, no errors
- [ ] RC, modes and e-stop switch respond correctly
- [ ] Arm for 30 s, disarm, pull the log
- [ ] `RPLD` about once a second, `PM.NLon` 0, `DSF.Dp` 0

## Flight profile

1. Take off in Alt Hold and hover at about 2 m for 30 s. Land and inspect
   if anything looks or sounds wrong.
2. Hover in Loiter for 30 s (Alt Hold if the heading is off).
3. Fly hard for 2 minutes in Acro or Stabilize, with at least 30 s above
   0.30 throttle: punch-outs, fast forward flight and turns. Log 15 spent
   only 11 s there.
4. Land by hand and disarm.

**Land now** on oscillation, a motor or ESC fault, an unexpected mode
change, any failsafe, an "Internal Error" message, or RC or video dropouts.

- [ ] Settings checked, rebooted
- [ ] Flown, log pulled

Log: `____` Pack: `____` Start/end V: `____ / ____`

## What to compare

References are log 15 (`031f2549`). It carried the PC sampler, which logs
12 and 14 suggest adds at most 0.3 points of load.

| Measure | Log field | Log 15 | Expect |
|---|---|---|---|
| Core1, throttle under 0.15 | `RPLD.C1` | 64.8% | lower |
| Core1, 0.15-0.20 | `RPLD.C1` | 65.6% | lower |
| Core1, 0.20-0.30 | `RPLD.C1` | 66.8% | lower |
| Core1, 0.30-0.45 | `RPLD.C1` | 67.8% | lower |
| Rate loop, worst in flight | `RTDT.dtMax` | 0.94 ms | no worse |
| Main loop | `PM.NLon`, `MaxT` | none, 5.46 ms | no long loops |
| Logging | `DSF.Dp` | 0 | no drops |
| Gyro attenuation, 150-250 Hz | batch FFT | -25 to -31 dB | within a few dB |
| Gyro attenuation, 250-400 Hz | batch FFT | -37 to -41 dB | within a few dB |
| Gyro attenuation, 400-1000 Hz | batch FFT | -42 to -44 dB | within a few dB |

The most the change can save is about 7-8 points of core1, the time the
trig calls took in flight. It will save less than that, by what the
polynomial itself costs.

```bash
python3 ../ardupilot-pr-analysis/32995/tools/ft_load_vs_throttle.py <log>
python3 .claude/skills/log-analyze/batch_fft.py <log> \
    --from-time <Acro start> --to-time <Acro end> --type gyro
```

The batch FFT reference used 205-330 s of log 15, in Acro. Its blocks are
not simultaneous, so read trends rather than tenths of a dB.

## Afterwards

- Keep the log, the `.elf` and this card's notes together.
- Record the result in `../ardupilot-pr-analysis/32995/`.
- If core1 falls and the notch holds, the two `Filter:` commits are generic
  and can go upstream as their own PR, without this card's commit.
- If core1 does not fall, the account of where the time goes is the first
  thing to re-check. `INS_HNTCH_OPTS` 18 against 22 is the other lever.
