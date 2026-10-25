# RP2350 branch and release maintenance

Notes for whoever keeps `ArduPilot-4.8-rp2350` and its releases. What the
branch contains, for users, is the top-level README's "What Is Included"
list; how the port works is in
`libraries/AP_HAL_ChibiOS/rp2350/DEVELOPMENT.md`.

## How the branch is built

The branch is linear, on top of ArduPilot master:

1. the commits of ArduPilot/ardupilot#32995, the RP2350 port, exactly as
   that PR's head has them;
2. the follow-up PRs in the README's order, each as its PR head has it,
   except where the README notes the branch carries an earlier version;
3. local commits with no PR yet, each of which says in the README where
   it is headed;
4. this repository's own commits: the top-level README and this
   directory, which never go upstream.

`modules/ChibiOS` points at ArduPilot/ChibiOS#113, the RP2350 ChibiOS
port, with ArduPilot/ChibiOS#115 (the microSD write #34477 needs) on top:
on 2026-10-25 that commit was the head of andyp1per/ChibiOS's
`rp2350-flight-test` branch and on no ArduPilot/ChibiOS branch. A fresh
clone still fetches it by SHA from ArduPilot/ChibiOS, because GitHub
serves objects across a fork network, but only while some branch in that
network keeps it: rewriting `rp2350-flight-test` can orphan the commit and
break every clone of a release. Before a release, make sure the pinned
commit is on a branch that will not be rewritten.

When a PR merges upstream, its commits arrive with the next rebase onto
master; drop them from the stack and mark the README entry as merged.
When a PR is updated, the branch keeps the version it has until the new
head is taken, and the README says which version it carries.

## Releases

Releases are GitHub releases on fossuav/ardupilot-rp2350, with prebuilt
Copter firmware for the boards that have flown (`BOARDS` in
`build_release.sh`: RPI_UAVFC and Pico2). Laurel is left to build from
source until it has flown on the current code.

```sh
# from a clean checkout of the commit to release
Tools/RP2350/build_release.sh rp2350-YYYYMMDD
```

The script refuses a tree with tracked changes, since the firmware reports
the commit it was built from. It builds each board, then stages the
`.apj`, `.uf2`, `.bin`, `_with_bl.hex` and ELF, the committed bootloader
`.uf2` and `.bin` from `Tools/bootloaders/`, `uploader.py`, and
`SHA256SUMS` in `build/rp2350-release/<tag>/`. Asset names carry the board but no version,
so the README's `releases/latest/download/<file>` links keep working. It
then prints the `gh release create` command; write the notes first:

- the commit, and what changed since the previous release;
- which flights the release, or the commits it adds, has had;
- anything a user upgrading needs to change.

Publishing is a separate, deliberate step: a release is public the moment
it is created, and `gh` creates the tag on GitHub without a local push.
