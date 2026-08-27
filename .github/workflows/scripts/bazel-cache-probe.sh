#!/usr/bin/env bash
#
# Diagnose why the GCS bazel remote cache gets ~0% cross-run hits.
#
# Background: the Linux Android build points bazel at a GCS remote cache, and
# the cache demonstrably works *within* a run (later backend builds hit
# hundreds of entries uploaded by earlier ones) but delivers almost nothing
# *across* runs -- three separate runs each reported exactly
# "4497 processes: 7 remote cache hit, 771 internal, 3719 local".
#
# Content-addressed inputs have already been ruled out from the CI logs: the
# linux/amd64 image manifest digest, the bazel-reported hashes of the
# unpinned archives, and the output_base path are all identical across runs.
# That leaves the parts of a bazel action key that are NOT file content --
# the command line, the action environment (PATH/LD_LIBRARY_PATH are
# inherited from the client unless --incompatible_strict_action_env is set)
# and the output paths.
#
# `bazel aquery --output=text` prints exactly those: every action's
# ActionKey, Environment and Command Line, and it is analysis-only, so a
# cross-run diff of it costs ~2 minutes instead of a 70-minute build.
#
# The probe collects, per run:
#   fingerprint.txt        host/env/toolchain identity (allowlisted vars only)
#   aquery-{a,b}.txt.gz    action keys + env + command lines
#   external-{a,b}.txt.gz  sha256 of every fetched external repo file
#   summary-{a,b}.txt      bazel's "N processes: ..." cache-hit line
#
# and reports two things:
#   same-run  : A vs B, with `bazel clean --expunge` in between, so every
#               external repo is re-fetched. Any diff here is fetch-time
#               non-determinism, caught in a single run.
#   cross-run : current bundle vs the previous run's bundle in GCS. Probe A's
#               hit rate IS the cross-run reuse number we care about.
#
set -euo pipefail

OUT="${PROBE_OUT_DIR:-/tmp/bazel-cache-probe}"
CANARY="${CANARY_TARGET:-//flutter/cpp:mlperf_driver}"
DIAG_PREFIX="${DIAG_GCS_PREFIX:?DIAG_GCS_PREFIX must be set}"
RUN_ID="${GITHUB_RUN_NUMBER:-local}"

BAZEL_ROOT_ARG="${BAZEL_OUTPUT_ROOT_ARG:-}"
BAZEL_CFG=(--config=android_arm64 --platforms=//platforms:android_arm64)

mkdir -p "$OUT"

section() {
  echo
  echo "=================================================================="
  echo "== $*"
  echo "=================================================================="
}

# ---------------------------------------------------------------- fingerprint
# Only an explicit allowlist of variable names is printed. The job deliberately
# does not define the signing/Firebase/BrowserStack secrets, but an allowlist
# is still the right default for something that gets uploaded to a bucket.
fingerprint() {
  local f="$OUT/fingerprint.txt"
  {
    echo "## host"
    uname -a
    echo "nproc=$(nproc)"
    echo "id=$(id -u):$(id -g)"

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
    for t in gcc g++ cc c++ ld ar python3 java bazel bazelisk; do
      p="$(command -v "$t" 2>/dev/null || true)"
      if [ -n "$p" ] && [ -f "$p" ]; then
        printf '%-9s %-40s %s\n' "$t" "$p" "$(sha256sum "$p" | cut -d' ' -f1)"
      else
        printf '%-9s %s\n' "$t" "<not found>"
      fi
    done
    echo "gcc -dumpversion: $(gcc -dumpversion 2>/dev/null || echo n/a)"
    echo "gcc -dumpmachine: $(gcc -dumpmachine 2>/dev/null || echo n/a)"

    echo
    echo "## gcc builtin include dirs (these land in @local_config_cc)"
    echo | gcc -E -xc++ - -v 2>&1 | sed -n '/#include <...> search starts here/,/End of search list/p' || true

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
  echo "wrote $f"
}

# ------------------------------------------------------------------- probes
run_aquery() {
  local tag="$1"
  echo "aquery ($tag) over deps($CANARY) ..."
  # shellcheck disable=SC2086
  bazel $BAZEL_ROOT_ARG aquery "${BAZEL_CFG[@]}" \
      "deps($CANARY)" --output=text \
      2> "$OUT/aquery-$tag.stderr" \
    | gzip -9 > "$OUT/aquery-$tag.txt.gz"
  echo "  $(zcat "$OUT/aquery-$tag.txt.gz" | grep -c '^  ActionKey:' || true) actions"
}

run_build() {
  local tag="$1" rc=0
  echo "build ($tag) $CANARY ..."
  # shellcheck disable=SC2086
  bazel $BAZEL_ROOT_ARG build ${BAZEL_CACHE_ARG:-} "${BAZEL_CFG[@]}" \
      "$CANARY" 2>&1 | tee "$OUT/build-$tag.log" || rc=$?
  rc="${PIPESTATUS[0]:-$rc}"
  grep -E '^INFO: [0-9]+ processes:' "$OUT/build-$tag.log" | tail -1 \
    > "$OUT/summary-$tag.txt" || true
  echo "  $(cat "$OUT/summary-$tag.txt" 2>/dev/null || echo '<no summary line>')"
  return "$rc"
}

# sha256 of every regular file in every fetched external repo. Symlinks are
# recorded by target rather than followed, so the NDK (symlinked in by
# android_ndk_repository) is identified without hashing gigabytes.
run_manifest() {
  local tag="$1" ob
  # shellcheck disable=SC2086
  ob="$(bazel $BAZEL_ROOT_ARG info output_base)"
  echo "manifest ($tag) of $ob/external ..."
  ( cd "$ob/external" && find . -type l -printf '%p -> %l\n' | LC_ALL=C sort ) \
    > "$OUT/symlinks-$tag.txt" 2>/dev/null || true
  ( cd "$ob/external" \
      && find . -type f -print0 \
      | xargs -0 -P "$(nproc)" -n 512 sha256sum 2>/dev/null ) \
    | LC_ALL=C sort -k2 | gzip -9 > "$OUT/external-$tag.txt.gz" || true
  echo "  $(zcat "$OUT/external-$tag.txt.gz" | wc -l) files, $(wc -l < "$OUT/symlinks-$tag.txt") symlinks"
}

# Counts objects bazel actually wrote. Two things are worth knowing here:
# whether uploads land at all (if they do not, no amount of key stability will
# help), and whether bazel keeps the path prefix of the --remote_cache URL --
# if it does not, the "probe" prefix silently shares a namespace with the real
# build cache at the bucket root.
count_cache_objects() {
  local when="$1" bucket="gs://mobile-app-build-290400_github-actions"
  local p
  for p in "$bucket/bazel-cache/probe" "$bucket/bazel-cache/linux-android" "$bucket"; do
    local ac cas
    ac="$(gsutil ls "$p/ac/" 2>/dev/null | wc -l || echo 0)"
    cas="$(gsutil ls "$p/cas/" 2>/dev/null | wc -l || echo 0)"
    printf '%-12s %-60s ac=%-8s cas=%s\n' "$when" "$p" "$ac" "$cas"
  done
}

# --------------------------------------------------------------- comparisons
# $1 label, $2 old file, $3 new file. Handles .gz transparently.
report_diff() {
  local label="$1" old="$2" new="$3" o n
  if [ ! -f "$old" ] || [ ! -f "$new" ]; then
    echo "-- $label: SKIPPED (missing $( [ -f "$old" ] || echo old )$( [ -f "$new" ] || echo ' new' ))"
    return 0
  fi
  o="$(mktemp)"; n="$(mktemp)"
  case "$old" in *.gz) zcat "$old" > "$o" ;; *) cp "$old" "$o" ;; esac
  case "$new" in *.gz) zcat "$new" > "$n" ;; *) cp "$new" "$n" ;; esac
  local count
  count="$(diff -U0 "$o" "$n" | grep -cE '^[+-][^+-]' || true)"
  if [ "$count" = "0" ]; then
    echo "-- $label: IDENTICAL"
  else
    echo "-- $label: $count differing lines (first 60):"
    diff -U0 "$o" "$n" | grep -E '^[+-][^+-]' | head -60
  fi
  rm -f "$o" "$n"
}

# aquery text output is one action per stanza; compare just the ActionKey set
# so a single changed key is obvious even when the surrounding text is huge.
report_action_keys() {
  local old="$1" new="$2" o n total_old total_new common
  if [ ! -f "$old" ] || [ ! -f "$new" ]; then
    echo "-- action keys: SKIPPED (no baseline)"
    return 0
  fi
  o="$(mktemp)"; n="$(mktemp)"
  zcat "$old" | grep -E '^  ActionKey: ' | sed 's/^  ActionKey: //' | LC_ALL=C sort -u > "$o"
  zcat "$new" | grep -E '^  ActionKey: ' | sed 's/^  ActionKey: //' | LC_ALL=C sort -u > "$n"
  total_old="$(wc -l < "$o")"; total_new="$(wc -l < "$n")"
  common="$(comm -12 "$o" "$n" | wc -l)"
  echo "-- action keys: $common of $total_new shared (baseline had $total_old)"
  if [ "$common" != "$total_new" ]; then
    echo "   NOT reproducible -- action keys changed. Sample changed actions:"
    # Show the stanzas whose keys are new, which carries Mnemonic/Env/CmdLine.
    comm -13 "$o" "$n" | head -3 | while read -r key; do
      echo "   ---- new key $key ----"
      zcat "$new" | grep -B6 -A14 "ActionKey: $key" | head -24
    done
  fi
  rm -f "$o" "$n"
}

# ------------------------------------------------------------------ main
section "fingerprint"
fingerprint
cat "$OUT/fingerprint.txt"

section "cache object counts BEFORE probe A"
count_cache_objects before | tee "$OUT/objects-before.txt"

section "probe A -- cold local state, reads whatever previous runs uploaded"
run_aquery a
run_build a || echo "WARNING: probe A build failed (rc=$?), continuing to gather evidence"
run_manifest a

section "cache object counts AFTER probe A -- did the uploads land?"
count_cache_objects after-a | tee "$OUT/objects-after-a.txt"

section "bazel clean --expunge (forces every external repo to be re-fetched)"
# shellcheck disable=SC2086
bazel $BAZEL_ROOT_ARG clean --expunge

section "probe B -- same run, same machine, re-fetched repos"
run_aquery b
run_build b || echo "WARNING: probe B build failed (rc=$?), continuing to gather evidence"
run_manifest b

section "SAME-RUN REPORT (A vs B) -- any diff here is fetch-time non-determinism"
report_action_keys "$OUT/aquery-a.txt.gz" "$OUT/aquery-b.txt.gz"
report_diff "external repo contents" "$OUT/external-a.txt.gz" "$OUT/external-b.txt.gz"
report_diff "external repo symlinks" "$OUT/symlinks-a.txt" "$OUT/symlinks-b.txt"
# Without --incompatible_strict_action_env, bazel inherits PATH (and
# LD_LIBRARY_PATH) from the client into every action's environment, and the
# action environment is part of the action key. If these blocks carry a value
# that differs between runners, no action can ever hit across runs.
echo "-- distinct action environments seen by aquery:"
zcat "$OUT/aquery-b.txt.gz" | grep -E '^  Environment: ' | LC_ALL=C sort -u | head -20

echo "-- probe A cache line: $(cat "$OUT/summary-a.txt" 2>/dev/null || echo n/a)"
echo "-- probe B cache line: $(cat "$OUT/summary-b.txt" 2>/dev/null || echo n/a)"
echo
echo "   Probe B reads what probe A just uploaded, so a high hit rate in B"
echo "   with a low one in A is the signature of the reported bug."

section "CROSS-RUN REPORT (previous run vs this run)"
prev="$(gsutil ls "$DIAG_PREFIX/" 2>/dev/null \
        | sed -n 's|.*/\([0-9][0-9]*\)/$|\1|p' \
        | LC_ALL=C sort -n \
        | awk -v cur="$RUN_ID" '($1+0) < (cur+0)' \
        | tail -1 || true)"
if [ -z "${prev:-}" ]; then
  echo "No previous probe bundle under $DIAG_PREFIX/ -- this is the baseline run."
  echo "Re-run this workflow to get the cross-run comparison."
else
  echo "Comparing against run $prev"
  mkdir -p "$OUT/prev"
  gsutil -m cp -r "$DIAG_PREFIX/$prev/*" "$OUT/prev/" >/dev/null 2>&1 || true
  report_diff "fingerprint (env/toolchain)" "$OUT/prev/fingerprint.txt" "$OUT/fingerprint.txt"
  report_action_keys "$OUT/prev/aquery-b.txt.gz" "$OUT/aquery-b.txt.gz"
  report_diff "external repo contents" "$OUT/prev/external-b.txt.gz" "$OUT/external-b.txt.gz"
  report_diff "external repo symlinks" "$OUT/prev/symlinks-b.txt" "$OUT/symlinks-b.txt"
  echo "-- previous run probe A cache line: $(cat "$OUT/prev/summary-a.txt" 2>/dev/null || echo n/a)"
  echo "-- this run     probe A cache line: $(cat "$OUT/summary-a.txt" 2>/dev/null || echo n/a)"
fi

section "upload this run's bundle"
gsutil -m cp "$OUT"/fingerprint.txt "$OUT"/summary-*.txt "$OUT"/symlinks-*.txt \
             "$OUT"/objects-*.txt "$OUT"/aquery-*.txt.gz "$OUT"/external-*.txt.gz \
             "$DIAG_PREFIX/$RUN_ID/" >/dev/null
echo "uploaded to $DIAG_PREFIX/$RUN_ID/"
