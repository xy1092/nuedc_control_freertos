#!/usr/bin/env bash
set -euo pipefail

DEV="${1:-/dev/ttyACM0}"
BAUD="${2:-115200}"

if [[ ! -e "$DEV" ]]; then
  echo "串口设备不存在: $DEV"
  exit 1
fi

exec picocom -b "$BAUD" "$DEV"
