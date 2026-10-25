#!/usr/bin/env bash
# Build the RP2350 release firmware and stage it for a GitHub release.
#
# usage: Tools/RP2350/build_release.sh TAG
#
# Builds Copter for each board in BOARDS from the checked-out commit and
# copies the images, the committed bootloaders, uploader.py (for users who
# have only the release) and a SHA256SUMS file into
# build/rp2350-release/TAG/. GitHub release assets are a flat list, so each
# file is prefixed with its board name; the names carry no version, which
# keeps the README's releases/latest/download links working across releases.

set -euo pipefail

BOARDS=(RPI_UAVFC Pico2)

if [ $# -ne 1 ] || [[ ! "$1" =~ ^[A-Za-z0-9._-]+$ ]]; then
    echo "usage: $0 TAG    (TAG may contain only letters, digits, '.', '_' and '-')" >&2
    exit 1
fi
TAG=$1

cd "$(git rev-parse --show-toplevel)"

# the firmware reports the commit it was built from, so build only a commit
if ! git diff --quiet HEAD --; then
    echo "tracked files have changes; commit or stash them first" >&2
    exit 1
fi
git submodule update --init --recursive

OUT=build/rp2350-release/$TAG
rm -rf "$OUT"
mkdir -p "$OUT"

for board in "${BOARDS[@]}"; do
    ./waf configure --board "$board"
    ./waf copter
    bin=build/$board/bin
    for f in arducopter.apj arducopter.uf2 arducopter.bin arducopter_with_bl.hex; do
        cp "$bin/$f" "$OUT/${board}_$f"
    done
    cp "$bin/arducopter" "$OUT/${board}_arducopter.elf"
    cp "Tools/bootloaders/${board}_bl.uf2" "Tools/bootloaders/${board}_bl.bin" "$OUT/"
done
cp Tools/scripts/uploader.py "$OUT/"

if command -v sha256sum >/dev/null; then
    sums=$(cd "$OUT" && sha256sum -- *)
else
    sums=$(cd "$OUT" && shasum -a 256 -- *)
fi
printf '%s\n' "$sums" > "$OUT/SHA256SUMS"

echo
echo "Staged $(git describe --always --dirty) for ${BOARDS[*]} in $OUT"
echo "Publish with:"
echo "  gh release create $TAG --repo fossuav/ardupilot-rp2350 --target $(git rev-parse HEAD) \\"
echo "      --title \"<title>\" --notes-file <notes.md> $OUT/*"
