#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/env.sh"

configure_only=0
if [[ "${1:-}" == "--configure-only" ]]; then
  configure_only=1
fi

if [[ -z "${MSPM0_SDK_ROOT:-}" || ! -f "$MSPM0_SDK_ROOT/.metadata/product.json" ]]; then
  echo "MSPM0 SDK not found. Set MSPM0_SDK_ROOT."
  exit 2
fi

mkdir -p "$BUILD_DIR/tmp"
export TMPDIR="${TMPDIR:-$BUILD_DIR/tmp}"

"$CMAKE_BIN" -S "$PROJECT_DIR" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DMSPM0_SDK_ROOT="$MSPM0_SDK_ROOT"

if [[ "$configure_only" -eq 1 ]]; then
  exit 0
fi

exec "$CMAKE_BIN" --build "$BUILD_DIR"
