#!/bin/bash
# Patch Bazel's wrapped_clang for macOS 26+ compatibility.
# macOS 26 requires LC_UUID in Mach-O binaries, but Bazel 6.x's
# wrapped_clang doesn't have it. This script triggers repo generation,
# then replaces the binary with a shell script equivalent.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
WRAPPER_SCRIPT="$SCRIPT_DIR/wrapped_clang.sh"

if [ ! -f "$WRAPPER_SCRIPT" ]; then
  echo "Error: $WRAPPER_SCRIPT not found" >&2
  exit 1
fi

OUTPUT_BASE=$(bazel info output_base 2>/dev/null)
CC_DIR="$OUTPUT_BASE/external/local_config_cc"

# Trigger repo generation if wrapped_clang doesn't exist yet
if [ ! -f "$CC_DIR/wrapped_clang" ]; then
  echo "Triggering Bazel repo generation..."
  bazel build --nobuild //flutter/cpp/flutter:libbackendbridge.so 2>/dev/null || true
fi

if [ ! -f "$CC_DIR/wrapped_clang" ]; then
  echo "Warning: wrapped_clang not found at $CC_DIR, skipping patch" >&2
  exit 0
fi

# Only patch if it's a Mach-O binary (not already a script)
if file "$CC_DIR/wrapped_clang" | grep -q "Mach-O"; then
  echo "Patching wrapped_clang for macOS 26+ compatibility..."
  cp "$WRAPPER_SCRIPT" "$CC_DIR/wrapped_clang"
  cp "$WRAPPER_SCRIPT" "$CC_DIR/wrapped_clang_pp"
  echo "Done."
fi
