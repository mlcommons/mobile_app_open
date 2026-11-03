#!/usr/bin/env bash

# Copyright 2026 The MLPerf Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

# Upload the Dart symbols of an Android build (the --split-debug-info folder)
# to Crashlytics with the Firebase CLI. Usage: upload-dart-symbols.sh <dir>
# Env: FIREBASE_ANDROID_APP_ID (required), FIREBASE_CLI (default: firebase).

set -euo pipefail

fail() {
  echo "upload-dart-symbols: $*" >&2
  exit 1
}

[ $# -eq 1 ] || fail "usage: $0 <symbols-dir>"
dir="$1"
[ -d "$dir" ] || fail "symbols directory not found: $dir"
[ -n "$(ls -A "$dir")" ] || fail "symbols directory is empty: $dir"
[ -n "${FIREBASE_ANDROID_APP_ID:-}" ] \
  || fail "FIREBASE_ANDROID_APP_ID must be set to upload Dart symbols to Crashlytics"
cli="${FIREBASE_CLI:-firebase}"
command -v "$cli" >/dev/null 2>&1 \
  || fail "Firebase CLI '$cli' not found on PATH (set FIREBASE_CLI or install firebase-tools)"

log="$(mktemp)"
trap 'rm -f "$log"' EXIT

status=0
"$cli" crashlytics:symbols:upload --app="$FIREBASE_ANDROID_APP_ID" "$dir" 2>&1 \
  | tee "$log" || status=$?
[ "$status" -eq 0 ] || fail "Firebase CLI exited with status $status"

# The standalone firebase-tools binary can exit 0 on internal errors (e.g. when
# it cannot set up its runtime), so also require the CLI's final success line
# (crashlytics-symbols-upload, of the version pinned in
# flutter/android/docker/Dockerfile).
grep -qF "Successfully uploaded all symbols" "$log" \
  || fail "Firebase CLI did not report a successful upload"
