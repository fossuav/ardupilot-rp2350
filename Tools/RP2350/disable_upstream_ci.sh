#!/usr/bin/env bash
# Turn off every GitHub Actions workflow on fossuav/ardupilot-rp2350 except
# this repository's own, .github/workflows/rp2350_*.
#
# usage: Tools/RP2350/disable_upstream_ci.sh [--dry-run]
#
# The tree carries upstream ArduPilot's .github/workflows unchanged, so the
# branch stays a plain stack of upstream PRs, and left enabled they run the
# whole upstream suite on every push. Disabling is a repository setting
# and lasts, but a workflow file new in a later master arrives enabled, so
# run this after each refresh.

set -euo pipefail

REPO=fossuav/ardupilot-rp2350
KEEP='^\.github/workflows/rp2350_'

dry=0
if [ "${1:-}" = "--dry-run" ]; then
    dry=1
fi

gh workflow list --repo "$REPO" --all --limit 200 --json id,path,state \
    --jq '.[] | select(.state == "active") | "\(.id) \(.path)"' |
while read -r id path; do
    if [[ "$path" =~ $KEEP ]]; then
        continue
    fi
    echo "disable $path"
    if [ $dry -eq 0 ]; then
        gh workflow disable "$id" --repo "$REPO"
    fi
done
