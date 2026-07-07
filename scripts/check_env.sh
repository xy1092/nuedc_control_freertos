#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

status=0

check_file() {
  local path="$1"
  local label="$2"
  if [[ -n "$path" && -e "$path" ]]; then
    printf '[OK]   %s: %s\n' "$label" "$path"
  else
    printf '[MISS] %s\n' "$label"
    status=1
  fi
}

check_dir() {
  local path="$1"
  local label="$2"
  if [[ -n "$path" && -d "$path" ]]; then
    printf '[OK]   %s: %s\n' "$label" "$path"
  else
    printf '[MISS] %s\n' "$label"
    status=1
  fi
}

echo '[tools]'
check_file "$CMAKE_BIN" 'cmake'
check_file "$NINJA_BIN" 'ninja'
check_file "$ARM_GCC_BIN" 'arm-none-eabi-gcc'
check_file "$ARM_GDB_BIN" 'arm-none-eabi-gdb'
check_file "$ARM_OBJCOPY_BIN" 'arm-none-eabi-objcopy'
check_file "$ARM_SIZE_BIN" 'arm-none-eabi-size'
check_file "${JLINK_EXE:-}" 'JLinkExe'
check_file "${JLINK_GDB_SERVER:-}" 'JLinkGDBServerCLExe'

echo
echo '[sdk]'
check_dir "${MSPM0_SDK_ROOT:-}" 'MSPM0 SDK'
if [[ -n "${MSPM0_SDK_ROOT:-}" ]]; then
  check_file "$MSPM0_SDK_ROOT/.metadata/product.json" 'MSPM0 product.json'
  check_file "$MSPM0_SDK_ROOT/source/ti/driverlib/dl_common.h" 'DriverLib headers'
fi

echo
echo '[project]'
check_file "$PROJECT_DIR/CMakeLists.txt" 'CMakeLists.txt'
check_file "$PROJECT_DIR/ti_msp_dl_config.c" 'ti_msp_dl_config.c'
check_file "$PROJECT_DIR/ti_msp_dl_config.h" 'ti_msp_dl_config.h'
check_file "$ELF_FILE" 'firmware ELF'
check_file "$BIN_FILE" 'firmware BIN'

echo
echo '[board]'
ls -l /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || echo 'No ttyACM/ttyUSB device'
lsusb 2>/dev/null | grep -Ei 'segger|j-link|jlink|texas instruments|xds110|mspm0|ti ' || echo 'No common debug probe detected'
id

exit "$status"
