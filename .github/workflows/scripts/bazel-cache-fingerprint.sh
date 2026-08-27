#!/usr/bin/env bash
#
# Record what a CI run feeds into bazel action keys, and diff it against the
# previous run of the same job.
#
# A shared remote cache only pays off if action keys reproduce across runs. When
# they do not, the useful question is "which input moved?", and answering it
# from build logs alone is guesswork. This writes a small fingerprint before the
# build and a content manifest of every fetched external repo after it, stores
# both in GCS keyed by run number, and prints a diff against the previous run.
#
# That is how the Linux cache bug was found: of ~240k fetched files exactly two
# differed, and of every environment value exactly one did -- PATH, carrying a
# per-run uuid that setup-gcloud puts there.
#
# The macOS jobs turned out NOT to have that problem -- the iOS build hits
# 4486 of 4486 cacheable actions across runs on different machines -- so the
# expensive external-repo manifest is off by default here. Set
# FINGERPRINT_MANIFEST=1 to turn it back on when a key really does drift.
#
# Usage:
#   bazel-cache-fingerprint.sh pre  <label>
#   bazel-cache-fingerprint.sh post <label>
#
# <label> namespaces the stored bundles, e.g. macos-android, macos-ios-tflite.
#
# Environment:
#   DIAG_GCS_PREFIX        gs:// prefix to store bundles under (required)
#   BAZEL_OUTPUT_ROOT_ARG  passed through to `bazel info output_base`
#   FINGERPRINT_OUT_DIR    where to write (default /tmp/bazel-fingerprint)
#
# Never fails the build: every step is best effort, and the script always exits
# 0. Diagnostics must not be able to break a release build.
#
set -uo pipefail

PHASE="${1:-}"
LABEL="${2:-default}"
OUT="${FINGERPRINT_OUT_DIR:-/tmp/bazel-fingerprint}"
PREFIX="${DIAG_GCS_PREFIX:-}"
RUN_ID="${GITHUB_RUN_NUMBER:-local}"
BAZEL_ROOT_ARG="${BAZEL_OUTPUT_ROOT_ARG:-}"

mkdir -p "$OUT"

section() {
  echo
  echo "------------------------------------------------------------------"
  echo "-- $*"
  echo "------------------------------------------------------------------"
}

# macOS has neither sha256sum nor nproc, and its find has no -printf.
sha_cmd() {
  if command -v sha256sum >/dev/null 2>&1; then
    echo "sha256sum"
  elif command -v shasum >/dev/null 2>&1; then
    echo "shasum -a 256"
  else
    echo ""
  fi
}
SHA="$(sha_cmd)"

ncpu() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  else
    sysctl -n hw.ncpu 2>/dev/null || echo 4
  fi
}

hash_one() {
  [ -n "$SHA" ] && [ -f "$1" ] && $SHA "$1" 2>/dev/null | cut -d' ' -f1
}

# ------------------------------------------------------------------- pre
# Only an explicit allowlist of variable names is recorded. The bundle is
# uploaded to a bucket, and these jobs do hold signing and Firebase secrets, so
# dumping the whole environment would be wrong.
write_fingerprint() {
  local f="$OUT/fingerprint.txt" v t p
  {
    echo "## host"
    uname -a
    echo "ncpu=$(ncpu)"
    command -v sw_vers >/dev/null 2>&1 && sw_vers

    echo
    echo "## runner image"
    # Set by the GitHub-hosted images; absent on self-hosted.
    echo "ImageOS=${ImageOS:-<unset>}"
    echo "ImageVersion=${ImageVersion:-<unset>}"
    echo "RUNNER_ARCH=${RUNNER_ARCH:-<unset>}"

    echo
    echo "## env (allowlist)"
    for v in PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH DYLD_FRAMEWORK_PATH \
             LIBRARY_PATH CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH \
             HOME LANG LC_ALL TZ SOURCE_DATE_EPOCH CC CXX CFLAGS CXXFLAGS \
             DEVELOPER_DIR SDKROOT MACOSX_DEPLOYMENT_TARGET \
             JAVA_HOME ANDROID_HOME ANDROID_SDK_ROOT ANDROID_NDK_HOME \
             ANDROID_NDK_ROOT ANDROID_NDK_LATEST_HOME FLUTTER_ROOT PUB_CACHE \
             GEM_HOME CP_HOME_DIR USER SHELL TMPDIR; do
      printf '%s=%s\n' "$v" "${!v-<unset>}"
    done

    echo
    echo "## toolchain identity (path + content hash)"
    for t in gcc g++ cc c++ clang clang++ ld ar libtool python3 java \
             bazel bazelisk protoc flutter dart pod cmake ninja; do
      p="$(command -v "$t" 2>/dev/null)"
      if [ -n "$p" ] && [ -f "$p" ]; then
        printf '%-10s %-52s %s\n' "$t" "$p" "$(hash_one "$p")"
      else
        printf '%-10s %s\n' "$t" "<not found>"
      fi
    done

    echo
    echo "## versions"
    command -v clang >/dev/null 2>&1 && clang --version 2>&1 | head -2
    command -v protoc >/dev/null 2>&1 && protoc --version 2>&1 | head -1
    command -v python3 >/dev/null 2>&1 && python3 --version 2>&1 | head -1
    command -v pod >/dev/null 2>&1 && pod --version 2>&1 | head -1
    command -v dart >/dev/null 2>&1 && dart --version 2>&1 | head -1

    if command -v xcodebuild >/dev/null 2>&1; then
      echo
      echo "## xcode"
      xcodebuild -version 2>&1 | head -4
      echo "xcode-select: $(xcode-select -p 2>/dev/null)"
      echo "iphoneos sdk: $(xcrun --sdk iphoneos --show-sdk-version 2>/dev/null)"
      echo "macosx  sdk: $(xcrun --sdk macosx --show-sdk-version 2>/dev/null)"
      echo "iphoneos sdk path: $(xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)"
    fi

    if command -v brew >/dev/null 2>&1; then
      echo
      echo "## brew formula versions"
      brew list --versions 2>/dev/null | LC_ALL=C sort
    fi

    echo
    echo "## android ndk"
    local ndk="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}"
    if [ -n "$ndk" ] && [ -d "$ndk" ]; then
      echo "ndk_home=$ndk"
      [ -f "$ndk/source.properties" ] && cat "$ndk/source.properties"
      local c
      for c in "$ndk"/toolchains/llvm/prebuilt/*/bin/clang; do
        [ -f "$c" ] && echo "clang $c $(hash_one "$c")"
      done
    else
      echo "no ANDROID_NDK_HOME/ANDROID_NDK_ROOT"
    fi
  } > "$f" 2>/dev/null
  echo "wrote $f"
  cat "$f"
}

# ------------------------------------------------------------------ post
# sha256 of every regular file bazel fetched. Symlinks are recorded by target
# rather than followed, so a symlinked NDK is identified without hashing it.
write_manifest() {
  local ob
  case "${FINGERPRINT_MANIFEST:-0}" in
    1|true|yes) ;;
    *)
      echo "manifest disabled (set FINGERPRINT_MANIFEST=1 to hash every fetched"
      echo "external repo file; it is worth minutes only when chasing a"
      echo "reproducibility bug)"
      return 0
      ;;
  esac
  # shellcheck disable=SC2086
  ob="$(bazel $BAZEL_ROOT_ARG info output_base 2>/dev/null)"
  if [ -z "$ob" ] || [ ! -d "$ob/external" ]; then
    echo "no external dir under '${ob:-<unknown output_base>}', skipping manifest"
    return 0
  fi
  echo "manifest of $ob/external"
  ( cd "$ob/external" 2>/dev/null && find . -type l -exec sh -c \
      'for l; do printf "%s -> %s\n" "$l" "$(readlink "$l")"; done' _ {} + 2>/dev/null ) \
    | LC_ALL=C sort > "$OUT/symlinks.txt" 2>/dev/null
  if [ -n "$SHA" ]; then
    ( cd "$ob/external" 2>/dev/null \
        && find . -type f -print0 2>/dev/null \
        | xargs -0 -P "$(ncpu)" -n 512 $SHA 2>/dev/null ) \
      | LC_ALL=C sort -k2 | gzip -9 > "$OUT/external.txt.gz" 2>/dev/null
  fi
  echo "  $( { gzip -dc "$OUT/external.txt.gz" 2>/dev/null || true; } | wc -l | tr -d ' ') files, $(wc -l < "$OUT/symlinks.txt" 2>/dev/null | tr -d ' ') symlinks"
}

# $1 label, $2 old, $3 new. Handles .gz transparently.
report_diff() {
  local label="$1"
  local old="$2"
  local new="$3"
  local o n count
  if [ ! -f "$old" ] || [ ! -f "$new" ]; then
    echo "-- $label: no baseline to compare against"
    return 0
  fi
  o="$(mktemp)"; n="$(mktemp)"
  case "$old" in *.gz) gzip -dc "$old" > "$o" 2>/dev/null ;; *) cp "$old" "$o" ;; esac
  case "$new" in *.gz) gzip -dc "$new" > "$n" 2>/dev/null ;; *) cp "$new" "$n" ;; esac
  count="$(diff -U0 "$o" "$n" 2>/dev/null | grep -cE '^[+-][^+-]')"
  [ -z "$count" ] && count=0
  if [ "$count" -eq 0 ]; then
    echo "-- $label: IDENTICAL"
  else
    echo "-- $label: $count differing lines (first 60)"
    diff -U0 "$o" "$n" 2>/dev/null | grep -E '^[+-][^+-]' | head -60
  fi
  rm -f "$o" "$n"
}

compare_with_previous() {
  [ -n "$PREFIX" ] || { echo "DIAG_GCS_PREFIX unset, skipping"; return 0; }
  command -v gsutil >/dev/null 2>&1 || { echo "no gsutil, skipping"; return 0; }

  local base="$PREFIX/$LABEL"
  local prev
  prev="$(gsutil ls "$base/" 2>/dev/null \
          | sed -n 's|.*/\([0-9][0-9]*\)/$|\1|p' \
          | LC_ALL=C sort -n \
          | awk -v cur="$RUN_ID" '($1+0) < (cur+0)' \
          | tail -1)"

  if [ -n "${prev:-}" ]; then
    echo "comparing against run $prev of $LABEL"
    mkdir -p "$OUT/prev"
    gsutil -m cp "$base/$prev/*" "$OUT/prev/" >/dev/null 2>&1
    report_diff "fingerprint (env / toolchain)" "$OUT/prev/fingerprint.txt" "$OUT/fingerprint.txt"
    report_diff "external repo contents" "$OUT/prev/external.txt.gz" "$OUT/external.txt.gz"
    report_diff "external repo symlinks" "$OUT/prev/symlinks.txt" "$OUT/symlinks.txt"
    echo
    echo "   Anything listed above is a candidate for breaking action keys."
    echo "   Bazel marker files (@name.marker) and .bzl files are NOT action"
    echo "   inputs; toolchain and source files are."
  else
    echo "no previous bundle under $base/ -- this run is the baseline"
  fi

  gsutil -m cp "$OUT/fingerprint.txt" "$OUT/symlinks.txt" "$OUT/external.txt.gz" \
               "$base/$RUN_ID/" >/dev/null 2>&1 \
    && echo "stored this run's bundle at $base/$RUN_ID/" \
    || echo "could not store this run's bundle"
}

case "$PHASE" in
  pre)
    section "bazel input fingerprint ($LABEL, run $RUN_ID)"
    write_fingerprint
    ;;
  post)
    section "bazel external repo manifest ($LABEL, run $RUN_ID)"
    write_manifest
    section "comparison with the previous run of $LABEL"
    compare_with_previous
    ;;
  *)
    echo "usage: $0 pre|post <label>" >&2
    ;;
esac

exit 0
