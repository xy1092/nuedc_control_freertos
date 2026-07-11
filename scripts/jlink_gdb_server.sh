#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

if [[ -z "${JLINK_GDB_SERVER:-}" || ! -x "$JLINK_GDB_SERVER" ]]; then
  echo "JLinkGDBServerCLExe not found."
  exit 2
fi

if [[ "$#" -gt 0 ]]; then
  exec "$JLINK_GDB_SERVER" "$@"
fi

exec "$JLINK_GDB_SERVER" \
  -device "$JLINK_DEVICE" \
  -if "$JLINK_IF" \
  -speed "$JLINK_SPEED_KHZ" \
  -port 2331 \
  -swoport 2332 \
  -telnetport 2333 \
  -singlerun \
  -nogui
