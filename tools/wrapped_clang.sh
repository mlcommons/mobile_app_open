#!/bin/bash
# Shell script replacement for Bazel's wrapped_clang binary.
# macOS 26 rejects the original Mach-O binary due to missing LC_UUID.
# This script replicates wrapped_clang's behavior:
#   1. Resolves __BAZEL_XCODE_SDKROOT__ and __BAZEL_XCODE_DEVELOPER_DIR__
#   2. Handles DEBUG_PREFIX_MAP_PWD argument
#   3. Invokes xcrun clang (or clang++) with processed args
set -euo pipefail

# Use APPLE_SDK_PLATFORM (set by Bazel) to resolve the correct SDK.
# e.g. "MacOSX" for macOS, "iPhoneOS" for iOS, "iPhoneSimulator" etc.
# xcrun requires lowercase SDK names (e.g. "iphoneos" not "iPhoneOS").
SDK_ARGS=()
if [ -n "${APPLE_SDK_PLATFORM:-}" ]; then
  sdk_name=$(printf '%s' "$APPLE_SDK_PLATFORM" | tr '[:upper:]' '[:lower:]')
  SDK_ARGS=(--sdk "$sdk_name")
fi

RESOLVED_SDKROOT=$(/usr/bin/xcrun "${SDK_ARGS[@]}" --show-sdk-path 2>/dev/null)
DEVELOPER_DIR=$(/usr/bin/xcode-select -p 2>/dev/null)

# Determine if we're wrapping clang or clang++
SELF=$(basename "$0")
if [[ "$SELF" == *"pp"* ]] || [[ "$SELF" == *"++"* ]]; then
  COMPILER="clang++"
else
  COMPILER="clang"
fi

args=()
for arg in "$@"; do
  # Replace SDK/Developer dir placeholders
  arg="${arg/__BAZEL_XCODE_SDKROOT__/$RESOLVED_SDKROOT}"
  arg="${arg/__BAZEL_XCODE_DEVELOPER_DIR__/$DEVELOPER_DIR}"

  # Handle DEBUG_PREFIX_MAP_PWD=<value> → -fdebug-prefix-map=<value>=.
  if [[ "$arg" == DEBUG_PREFIX_MAP_PWD=* ]]; then
    val="${arg#DEBUG_PREFIX_MAP_PWD=}"
    args+=("-fdebug-prefix-map=${val}=.")
    continue
  fi

  args+=("$arg")
done

exec /usr/bin/xcrun "${SDK_ARGS[@]}" "$COMPILER" "${args[@]}"
