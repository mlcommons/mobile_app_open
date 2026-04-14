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
# Note: match "_pp" not "pp" — "wrapped" itself contains "pp".
SELF=$(basename "$0")
if [[ "$SELF" == *"_pp"* ]] || [[ "$SELF" == *"++"* ]]; then
  COMPILER="clang++"
else
  COMPILER="clang"
fi

process_arg() {
  local arg="$1"
  arg="${arg/__BAZEL_XCODE_SDKROOT__/$RESOLVED_SDKROOT}"
  arg="${arg/__BAZEL_XCODE_DEVELOPER_DIR__/$DEVELOPER_DIR}"
  if [[ "$arg" == DEBUG_PREFIX_MAP_PWD=* ]]; then
    local val="${arg#DEBUG_PREFIX_MAP_PWD=}"
    printf '%s\n' "-fdebug-prefix-map=${val}=."
  else
    printf '%s\n' "$arg"
  fi
}

args=()
for arg in "$@"; do
  # Handle @params files: read, process, and rewrite in-place
  if [[ "$arg" == @* ]]; then
    params_file="${arg#@}"
    if [ -f "$params_file" ]; then
      tmp_params=$(mktemp)
      while IFS= read -r line || [ -n "$line" ]; do
        process_arg "$line" >> "$tmp_params"
      done < "$params_file"
      mv "$tmp_params" "$params_file"
      args+=("$arg")
      continue
    fi
  fi

  args+=("$(process_arg "$arg")")
done

exec /usr/bin/xcrun "${SDK_ARGS[@]}" "$COMPILER" "${args[@]}"
