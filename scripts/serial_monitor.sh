#!/usr/bin/env bash
set -euo pipefail

DEV="${1:-/dev/ttyACM0}"
BAUD="${2:-115200}"

if [[ ! -e "$DEV" ]]; then
  echo "串口设备不存在: $DEV"
  exit 1
fi

if command -v picocom >/dev/null 2>&1; then
  exec picocom -b "$BAUD" "$DEV"
fi

if command -v minicom >/dev/null 2>&1; then
  exec minicom -D "$DEV" -b "$BAUD"
fi

echo "请安装 picocom 或 minicom。"
exit 2
