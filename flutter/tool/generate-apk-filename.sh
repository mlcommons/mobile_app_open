#!/usr/bin/env bash
# Generate a APK file name based on build env

set -e

suffix=""

if [ "${WITH_QTI}" = 1 ]; then suffix+="q"; fi
if [ "${WITH_SAMSUNG}" = 1 ]; then suffix+="s"; fi
if [ "${WITH_MEDIATEK}" = 1 ]; then suffix+="m"; fi
if [ "${WITH_PIXEL}" = 1 ]; then suffix+="g"; fi
if [ "${WITH_TFLITE}" = 1 ]; then suffix+="t"; fi

today=$(date +%F)
commit=$(git rev-parse --short HEAD)

output="${today}_mlperfbench-${commit}-${suffix}.apk"

echo "${output}"
