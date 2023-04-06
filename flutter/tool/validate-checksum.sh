#!/bin/sh

set -e

while getopts "d:f:" flag; do
  case "${flag}" in
  d) filedir=${OPTARG} ;;
  f) checksumfile=${OPTARG} ;;
  ?) exit 1 ;;
  esac
done

# based on https://unix.stackexchange.com/a/421617
while read -r checksum filename; do
  if [ ! -f "$filedir/$filename" ]; then
    printf 'FAIL: %s (File not found)\n' "$filename"
    exit 1
  fi

  shasum -a 256 "$filedir/$filename" | {
    read -r realsum name
    if [ "$realsum" != "$checksum" ]; then
      printf 'FAIL: %s (mismatched checksum: %s != %s)\n' "$filename" "$checksum" "$realsum"
      exit 1
    else
      printf 'PASS: %s \n' "$filename"
    fi
  }
done <"$checksumfile" >&2
