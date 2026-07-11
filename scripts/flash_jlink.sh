#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

image_file="${1:-$BIN_FILE}"

if [[ -z "${JLINK_EXE:-}" || ! -x "$JLINK_EXE" ]]; then
  echo '未找到 JLinkExe。'
  echo '请确认 SEGGER J-Link Software Pack 已安装，或设置 JLINK_EXE。'
  exit 2
fi

if [[ ! -f "$image_file" ]]; then
  echo "未找到待烧录固件: $image_file"
  echo '请先完成 Build。'
  exit 2
fi

mkdir -p "$BUILD_DIR/tmp"
cmd_file="$(mktemp "$BUILD_DIR/tmp/jlink_cmd.XXXXXX")"
trap 'rm -f "$cmd_file"' EXIT

case "$image_file" in
  *.bin)
    load_cmd="loadfile $image_file, $JLINK_FLASH_ADDR"
    ;;
  *)
    load_cmd="loadfile $image_file"
    ;;
esac

cat >"$cmd_file" <<EOF
r
h
$load_cmd
r
g
q
EOF

set +e
jlink_output="$("$JLINK_EXE" \
  -device "$JLINK_DEVICE" \
  -if "$JLINK_IF" \
  -speed "$JLINK_SPEED_KHZ" \
  -autoconnect 1 \
  -NoGui 1 \
  -CommandFile "$cmd_file" 2>&1)"
jlink_status=$?
set -e

printf '%s\n' "$jlink_output"

if [[ "$jlink_status" -ne 0 ]] ||
   ! grep -Fq 'J-Link: Flash download:' <<<"$jlink_output" ||
   ! grep -Fxq 'O.K.' <<<"$jlink_output"; then
  echo 'J-Link 烧录失败：未能连接或下载到目标芯片。'
  exit 1
fi

echo 'J-Link 烧录成功：固件已完成编程和校验。'
