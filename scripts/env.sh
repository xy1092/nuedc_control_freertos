#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

export CMAKE_BIN="${CMAKE_BIN:-$(command -v cmake || true)}"
export NINJA_BIN="${NINJA_BIN:-$(command -v ninja || true)}"
export ARM_GCC_BIN="${ARM_GCC_BIN:-$(command -v arm-none-eabi-gcc || true)}"
export ARM_GDB_BIN="${ARM_GDB_BIN:-$(command -v arm-none-eabi-gdb || true)}"
export ARM_OBJCOPY_BIN="${ARM_OBJCOPY_BIN:-$(command -v arm-none-eabi-objcopy || true)}"
export ARM_SIZE_BIN="${ARM_SIZE_BIN:-$(command -v arm-none-eabi-size || true)}"
export JLINK_EXE="${JLINK_EXE:-$(command -v JLinkExe || true)}"
export JLINK_GDB_SERVER="${JLINK_GDB_SERVER:-$(command -v JLinkGDBServerCLExe || true)}"

if [[ -z "${MSPM0_SDK_ROOT:-}" ]]; then
  for sdk_dir in \
    "$HOME/ti/mspm0_sdk_2_10_00_04/mspm0_sdk_2_10_00_04" \
    "$HOME/ti/mspm0_sdk_2_10_00_04" \
    "$HOME/ti/mspm0_sdk_2_07_00_05/mspm0_sdk_2_07_00_05" \
    "$HOME/ti/mspm0_sdk_2_07_00_05" \
    "/opt/ti/mspm0_sdk"
  do
    if [[ -f "$sdk_dir/.metadata/product.json" ]]; then
      export MSPM0_SDK_ROOT="$sdk_dir"
      break
    fi
  done
fi

if [[ -z "${JLINK_EXE:-}" ]]; then
  for jlink_dir in \
    "$HOME/Downloads/JLink_Linux_V938_x86_64" \
    "$HOME/下载/JLink_Linux_V938_x86_64" \
    "$HOME/JLink_Linux_V938_x86_64" \
    "/opt/SEGGER/JLink"
  do
    if [[ -x "$jlink_dir/JLinkExe" ]]; then
      export JLINK_EXE="$jlink_dir/JLinkExe"
      break
    fi
  done
fi

if [[ -z "${JLINK_GDB_SERVER:-}" && -n "${JLINK_EXE:-}" ]]; then
  jlink_dir="$(cd "$(dirname "$JLINK_EXE")" && pwd)"
  for gdb_server in \
    "$jlink_dir/JLinkGDBServerCLExe" \
    "$jlink_dir/JLinkGDBServerExe" \
    "$jlink_dir/JLinkGDBServer"
  do
    if [[ -x "$gdb_server" ]]; then
      export JLINK_GDB_SERVER="$gdb_server"
      break
    fi
  done
fi

export BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build-fw}"
export ELF_FILE="${ELF_FILE:-$BUILD_DIR/nuedc_control_freertos.out}"
export BIN_FILE="${BIN_FILE:-$BUILD_DIR/nuedc_control_freertos.bin}"
export HEX_FILE="${HEX_FILE:-$BUILD_DIR/nuedc_control_freertos.hex}"
export CMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE:-$PROJECT_DIR/cmake/toolchains/arm-none-eabi-gcc.cmake}"
export JLINK_DEVICE="${JLINK_DEVICE:-MSPM0G3507}"
export JLINK_SPEED_KHZ="${JLINK_SPEED_KHZ:-1000}"
