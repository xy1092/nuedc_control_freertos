#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build-fw}"
CMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE:-$PROJECT_DIR/cmake/toolchains/arm-none-eabi-gcc.cmake}"

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

mkdir -p "$BUILD_DIR"

cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DMSPM0_SDK_ROOT="$MSPM0_SDK_ROOT"
cmake --build "$BUILD_DIR"
