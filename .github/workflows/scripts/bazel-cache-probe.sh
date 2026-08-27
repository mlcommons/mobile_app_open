#!/usr/bin/env bash
#
# Controlled experiment for the bazel remote cache hit rate.
#
# Background. The Linux Android build points bazel at a GCS remote cache. It
# worked within a run (later backend builds hit hundreds of entries uploaded by
# earlier ones) and delivered nothing across runs -- three separate runs each
# reported the identical "4497 processes: 7 remote cache hit, 771 internal,
# 3719 local" for the first and most expensive build step.
#
# Earlier revisions of this probe established, by measurement:
#   * uploads land -- 1189 locally executed actions produced exactly ac=1189
#     objects in a prefix that started empty
#   * objects persist -- the real cache holds ~19k action entries
#   * bazel keeps the path prefix of the --remote_cache URL (bucket root stayed
#     at ac=0)
#   * a full re-fetch of every external repo inside one run still hit 1188 of
#     1189 actions, so fetching is reproducible
#   * across two runs, the only differing external repo file was loadgen's
#     version_generated.cc, and the only differing environment value was PATH:
#       PATH=/__w/_temp/447e3f0f-.../google-cloud-sdk/bin:...
#       PATH=/__w/_temp/ada2e556-.../google-cloud-sdk/bin:...
#     setup-gcloud installs the SDK into a fresh $RUNNER_TEMP/<uuid> every run.
#
# Bazel bakes the client PATH into every action's environment unless
# --incompatible_strict_action_env is set, and the action environment is part of
# every action key -- so a new uuid on PATH invalidated the entire cache on
# every run.
#
# This probe now tests that claim and its fix directly. Four builds of one small
# canary target, resetting all bazel state between them, varying only PATH:
#
#   a  PATH prefix A, --noincompatible_strict_action_env   populates
#   b  PATH prefix B, --noincompatible_strict_action_env   expect ~0% hits
#   c  PATH prefix C, strict action env (from .bazelrc)    populates
#   d  PATH prefix D, strict action env (from .bazelrc)    expect ~100% hits
#
# b near zero reproduces the bug; d near 100% proves the fix.
#
set -euo pipefail

OUT="${PROBE_OUT_DIR:-/tmp/bazel-cache-probe}"
CANARY="${CANARY_TARGET:-//flutter/cpp:mlperf_driver}"
DIAG_PREFIX="${DIAG_GCS_PREFIX:?DIAG_GCS_PREFIX must be set}"
BUCKET="${DIAG_BUCKET:-gs://mobile-app-build-290400_github-actions}"
CACHE_BASE="${DIAG_CACHE_BASE:-https://storage.googleapis.com/mobile-app-build-290400_github-actions/bazel-cache}"
RUN_ID="${GITHUB_RUN_NUMBER:-local}"

BAZEL_ROOT_ARG="${BAZEL_OUTPUT_ROOT_ARG:-}"
BAZEL_CFG=(--config=android_arm64 --platforms=//platforms:android_arm64)
ORIG_PATH="$PATH"

mkdir -p "$OUT"

# The bundle is what the next run compares against, so upload it even when a
# later step dies -- an earlier probe run lost its baseline exactly that way.
upload_bundle() {
  local rc=$?
  echo
  echo "== uploading this run's bundle to $DIAG_PREFIX/$RUN_ID/ (script rc=$rc)"
  gsutil -m cp "$OUT"/fingerprint.txt "$OUT"/summary-*.txt "$OUT"/objects-*.txt \
               "$OUT"/external-*.txt.gz "$DIAG_PREFIX/$RUN_ID/" 2>/dev/null \
    && echo "   uploaded" || echo "   upload incomplete (some files may not exist)"
  return "$rc"
}
trap upload_bundle EXIT

section() {
  echo
  echo "=================================================================="
  echo "== $*"
  echo "=================================================================="
}

# ---------------------------------------------------------------- fingerprint
# Only an explicit allowlist of variable names is printed; this file is uploaded
# to a bucket, so an allowlist is the right default even though this job does
# not define the signing/Firebase/BrowserStack secrets.
fingerprint() {
  local f="$OUT/fingerprint.txt"
  {
    echo "## host"
    uname -a
    echo "nproc=$(nproc)"

    echo
    echo "## env (allowlist)"
    local v
    for v in PATH LD_LIBRARY_PATH LIBRARY_PATH CPATH C_INCLUDE_PATH \
             CPLUS_INCLUDE_PATH HOME LANG LC_ALL TZ SOURCE_DATE_EPOCH \
             CC CXX CFLAGS CXXFLAGS JAVA_HOME ANDROID_HOME ANDROID_SDK_ROOT \
             ANDROID_NDK_HOME ANDROID_NDK_ROOT FLUTTER_ROOT PUB_CACHE \
             USER SHELL TMPDIR; do
      printf '%s=%s\n' "$v" "${!v-<unset>}"
    done

    echo
    echo "## host toolchain identity"
    local t p
    for t in gcc g++ cc c++ ld ar python3 java bazel; do
      p="$(command -v "$t" 2>/dev/null || true)"
      if [ -n "$p" ] && [ -f "$p" ]; then
        printf '%-9s %-40s %s\n' "$t" "$p" "$(sha256sum "$p" | cut -d' ' -f1)"
      else
        printf '%-9s %s\n' "$t" "<not found>"
      fi
    done
    echo "gcc -dumpversion: $(gcc -dumpversion 2>/dev/null || echo n/a)"

    echo
    echo "## android ndk identity"
    local ndk="${ANDROID_NDK_HOME:-}"
    if [ -n "$ndk" ] && [ -d "$ndk" ]; then
      echo "ndk_home=$ndk"
      [ -f "$ndk/source.properties" ] && cat "$ndk/source.properties"
      local clang="$ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/clang"
      [ -f "$clang" ] && echo "clang sha256=$(sha256sum "$clang" | cut -d' ' -f1)"
    else
      echo "ANDROID_NDK_HOME unset or missing"
    fi
  } > "$f"
  cat "$f"
}

# Counting objects proves uploads land and shows whether bazel keeps the path
# prefix of the --remote_cache URL (if it did not, everything would pile up at
# the bucket root).
count_cache_objects() {
  local when="$1" p ac cas
  for p in "$BUCKET/bazel-cache/probe-nostrict" "$BUCKET/bazel-cache/probe-strict" \
           "$BUCKET/bazel-cache/linux-android" "$BUCKET"; do
    ac="$(gsutil ls "$p/ac/" 2>/dev/null | wc -l || echo 0)"
    cas="$(gsutil ls "$p/cas/" 2>/dev/null | wc -l || echo 0)"
    printf '%-10s %-62s ac=%-8s cas=%s\n' "$when" "$p" "$ac" "$cas"
  done
}

# `bazel clean --expunge` is unusable here: it gives up with a FATAL when the
# server outlives its SIGKILL grace period, which is what happened on the first
# probe run (exit 36) while the server was still flushing cache uploads. Tear
# the output base down by hand, keeping --output_user_root identical so every
# probe sees exactly the same absolute paths.
reset_bazel_state() {
  local ob=""
  # shellcheck disable=SC2086
  ob="$(bazel $BAZEL_ROOT_ARG info output_base 2>/dev/null || true)"
  # shellcheck disable=SC2086
  bazel $BAZEL_ROOT_ARG shutdown >/dev/null 2>&1 || true
  sleep 5
  pkill -9 -f 'bazel.*server' >/dev/null 2>&1 || true
  pkill -9 -f 'bazel_real' >/dev/null 2>&1 || true
  sleep 3
  if [ -n "$ob" ] && [ -d "$ob" ]; then
    rm -rf "$ob"
    echo "removed output base $ob"
  fi
}

# run_probe <tag> <path-prefix-dir> <cache-prefix> [extra bazel flags...]
run_probe() {
  local tag="$1" pathdir="$2" cacheprefix="$3"; shift 3
  local rc=0
  mkdir -p "$pathdir"
  export PATH="$pathdir:$ORIG_PATH"

  section "probe $tag -- PATH prefix $pathdir, cache prefix $cacheprefix, flags: $*"
  echo "PATH=$PATH"
  # shellcheck disable=SC2086
  bazel $BAZEL_ROOT_ARG build \
      "--remote_cache=$CACHE_BASE/$cacheprefix" --google_default_credentials \
      "$@" "${BAZEL_CFG[@]}" "$CANARY" 2>&1 | tee "$OUT/build-$tag.log" || rc=$?
  grep -E '^INFO: [0-9]+ processes:' "$OUT/build-$tag.log" | tail -1 \
    > "$OUT/summary-$tag.txt" 2>/dev/null || true
  echo "probe $tag: $(cat "$OUT/summary-$tag.txt" 2>/dev/null || echo '<no summary line>')"
  export PATH="$ORIG_PATH"
  return 0
}

# sha256 of every regular file in every fetched external repo. Symlinks are
# recorded by target rather than followed, so the NDK (symlinked in by
# android_ndk_repository) is identified without hashing gigabytes.
run_manifest() {
  local tag="$1" ob
  # shellcheck disable=SC2086
  ob="$(bazel $BAZEL_ROOT_ARG info output_base 2>/dev/null || true)"
  [ -n "$ob" ] && [ -d "$ob/external" ] || { echo "no external dir for $tag"; return 0; }
  ( cd "$ob/external" \
      && find . -type f -print0 \
      | xargs -0 -P "$(nproc)" -n 512 sha256sum 2>/dev/null ) \
    | LC_ALL=C sort -k2 | gzip -9 > "$OUT/external-$tag.txt.gz" || true
  echo "manifest $tag: $(zcat "$OUT/external-$tag.txt.gz" | wc -l) files"
}

# $1 label, $2 old file, $3 new file. Handles .gz transparently.
report_diff() {
  local label="$1" old="$2" new="$3" o n count
  if [ ! -f "$old" ] || [ ! -f "$new" ]; then
    echo "-- $label: SKIPPED (missing baseline)"
    return 0
  fi
  o="$(mktemp)"; n="$(mktemp)"
  case "$old" in *.gz) zcat "$old" > "$o" ;; *) cp "$old" "$o" ;; esac
  case "$new" in *.gz) zcat "$new" > "$n" ;; *) cp "$new" "$n" ;; esac
  count="$(diff -U0 "$o" "$n" | grep -cE '^[+-][^+-]' || true)"
  if [ "$count" = "0" ]; then
    echo "-- $label: IDENTICAL"
  else
    echo "-- $label: $count differing lines (first 40):"
    diff -U0 "$o" "$n" | grep -E '^[+-][^+-]' | head -40 || true
  fi
  rm -f "$o" "$n"
}

# Pull the hit count out of a bazel summary line, as a fraction of the actions
# that can actually be cached. Bazel's "N internal" actions (symlink trees,
# workspace status, ...) never go to the remote cache, so counting them in the
# denominator understates a perfect result -- 1189 hits with 229 internal and 0
# local is 100% reuse, not 83%.
hit_rate() {
  local f="$1" line hits total internal cacheable
  [ -f "$f" ] || { echo "n/a"; return 0; }
  line="$(cat "$f")"
  total="$(sed -E 's/^INFO: ([0-9]+) processes:.*/\1/' <<<"$line")"
  hits="$(grep -oE '[0-9]+ remote cache hit' <<<"$line" | grep -oE '^[0-9]+' || true)"
  internal="$(grep -oE '[0-9]+ internal' <<<"$line" | grep -oE '^[0-9]+' || true)"
  [ -z "$hits" ] && hits=0
  [ -z "$internal" ] && internal=0
  if [ -n "$total" ] && [ "$total" -gt 0 ] 2>/dev/null; then
    cacheable=$(( total - internal ))
    if [ "$cacheable" -gt 0 ]; then
      echo "$hits/$cacheable cacheable ($(( hits * 100 / cacheable ))%)"
    else
      echo "$hits/0 cacheable"
    fi
  else
    echo "$hits/?"
  fi
}

# ------------------------------------------------------------------ main
section "fingerprint"
fingerprint

section "cache object counts BEFORE"
count_cache_objects before | tee "$OUT/objects-before.txt"

# --- leg 1: PATH varies, action env NOT pinned. Reproduces the bug. ---
reset_bazel_state
run_probe a /tmp/probe-path-a probe-nostrict --noincompatible_strict_action_env
run_manifest a

reset_bazel_state
run_probe b /tmp/probe-path-b probe-nostrict --noincompatible_strict_action_env
run_manifest b

# --- leg 2: PATH varies, action env pinned by .bazelrc. Tests the fix. ---
reset_bazel_state
run_probe c /tmp/probe-path-c probe-strict

reset_bazel_state
run_probe d /tmp/probe-path-d probe-strict

section "cache object counts AFTER"
count_cache_objects after | tee "$OUT/objects-after.txt"

section "RESULT"
echo "  a (unpinned env, PATH A, populates) : $(hit_rate "$OUT/summary-a.txt")"
echo "  b (unpinned env, PATH B)            : $(hit_rate "$OUT/summary-b.txt")   <- expect ~0%"
echo "  c (pinned env,   PATH C, populates) : $(hit_rate "$OUT/summary-c.txt")"
echo "  d (pinned env,   PATH D)            : $(hit_rate "$OUT/summary-d.txt")   <- expect ~100%"
echo
echo "  Only PATH differs between a and b, and between c and d. A low b with a"
echo "  high d means PATH alone was invalidating every action key, and that"
echo "  --incompatible_strict_action_env fixes it."
echo
report_diff "external repo contents across a re-fetch (a vs b)" \
            "$OUT/external-a.txt.gz" "$OUT/external-b.txt.gz"
echo "  (loadgen's version_generated.cc used to be the one file that differed"
echo "   here; WORKSPACE now pins its build-date stamps.)"

section "CROSS-RUN COMPARISON (previous run vs this run)"
prev="$(gsutil ls "$DIAG_PREFIX/" 2>/dev/null \
        | sed -n 's|.*/\([0-9][0-9]*\)/$|\1|p' \
        | LC_ALL=C sort -n \
        | awk -v cur="$RUN_ID" '($1+0) < (cur+0)' \
        | tail -1 || true)"
if [ -z "${prev:-}" ]; then
  echo "No previous bundle under $DIAG_PREFIX/ -- this is the baseline run."
else
  echo "Comparing against run $prev"
  mkdir -p "$OUT/prev"
  gsutil -m cp -r "$DIAG_PREFIX/$prev/*" "$OUT/prev/" >/dev/null 2>&1 || true
  report_diff "fingerprint (env/toolchain)" "$OUT/prev/fingerprint.txt" "$OUT/fingerprint.txt"
  report_diff "external repo contents" "$OUT/prev/external-a.txt.gz" "$OUT/external-a.txt.gz"
  echo
  echo "-- previous run probe c: $(hit_rate "$OUT/prev/summary-c.txt")"
  echo "-- this run     probe c: $(hit_rate "$OUT/summary-c.txt")"
  echo "   Probe c reads a prefix the previous run filled with pinned-env keys,"
  echo "   so a high number here is cross-run reuse actually working."
fi
