#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
source "$SCRIPT_DIR/env.sh"

TEST_BUILD_DIR="${LINE_TEST_BUILD_DIR:-$PROJECT_DIR/build-line-sensor-test}"

cmake -S "$PROJECT_DIR" -B "$TEST_BUILD_DIR" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
  -DMSPM0_SDK_ROOT="$MSPM0_SDK_ROOT" \
  -DNUEDC_TEST_LINE_SENSOR=ON
cmake --build "$TEST_BUILD_DIR"
