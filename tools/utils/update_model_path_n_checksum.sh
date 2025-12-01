#!/usr/bin/env bash
set -euo pipefail

# Update model_path and model_checksum in *.pbtxt files under the given directory.
# - Replaces model_path values that start with SOURCE_URL with TARGET_URL.
# - Sets model_checksum from checksums.txt by matching the relative path (preferred) or the basename as fallback.
# - Checksums file formats supported:
#     MD5 (<path or filename>) = <md5>
#     <hash><space><path or filename>  (legacy: md5 or sha256)
#
# Usage:
#   update_model_path_n_checksum.sh [-d DIR] [-c CHECKSUM_FILE] [-t TARGET_URL] [-s SOURCE_URL]
#     -d DIR            Directory to scan (default: script directory)
#     -c CHECKSUM_FILE  Path to checksums.txt (default: DIR/checksums.txt)
#     -t|--target-url   Base target URL (default: https://mobile.mlcommons-storage.org/app-resources/models/v5_0_1/)
#     -s|--source-url   Source URL prefix to replace (default: local:///mlperf_models/)
#
#

SCAN_DIR="$(cd "$(dirname "$0")" && pwd)"
TARGET_URL="https://mobile.mlcommons-storage.org/app-resources/models/v5_0_1/"
SOURCE_URL="local:///mlperf_models/"
CHECKSUM_FILE=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    -d) SCAN_DIR="$2"; shift 2 ;;
    -c) CHECKSUM_FILE="$2"; shift 2 ;;
    -t|--target-url) TARGET_URL="$2"; shift 2 ;;
    -s|--source-url) SOURCE_URL="$2"; shift 2 ;;
    -h|--help)
      grep '^#' "$0" | sed -E 's/^# ?//' | sed -n '1,40p'
      exit 0
      ;;
    *) echo "Unknown option: $1" >&2; exit 1 ;;
  esac
done

# Normalize URLs to have a trailing slash for concatenation
if [[ -n "${TARGET_URL}" && "${TARGET_URL}" != */ ]]; then TARGET_URL="${TARGET_URL}/"; fi
if [[ -n "${SOURCE_URL}" && "${SOURCE_URL}" != */ ]]; then SOURCE_URL="${SOURCE_URL}/"; fi

if [[ -z "${CHECKSUM_FILE}" ]]; then
  CHECKSUM_FILE="$SCAN_DIR/checksums.txt"
fi

if [[ ! -f "$CHECKSUM_FILE" ]]; then
  echo "ERROR: checksum file not found: $CHECKSUM_FILE" >&2
  exit 1
fi

if [[ ! -d "$SCAN_DIR" ]]; then
  echo "ERROR: directory not found: $SCAN_DIR" >&2
  exit 1
fi


shopt -s nullglob
# Gather files using glob to avoid subshell side-effects
files=("$SCAN_DIR"/*.pbtxt)
if [[ ! -e "${files[0]}" ]]; then
  echo "No .pbtxt files found in $SCAN_DIR"
  exit 0
fi

for f in "${files[@]}"; do
  tmp="${f}.tmp.$$"
  changes_in_file=0

  # Process with awk reading checksums first, then the pbtxt file
  awk -v TARGET_URL="$TARGET_URL" -v SOURCE_URL="$SOURCE_URL" '
    BEGIN {
      local_prefix = SOURCE_URL
    }
    # Phase 1: Read checksum file (first input)
    FNR==NR {
      # Accepted formats per line:
      #   MD5 (<path or filename>) = <md5>
      #   <hash><space><path or filename>   (legacy; md5 or sha256)
      # Skip empty/comment lines
      if ($0 ~ /^[ \t]*$/) next;
      if ($0 ~ /^[#]/) next;

      line=$0
      hash=""; path="";

      # Try to parse: MD5 (path) = hash
      lp = index(line, "(");
      rp = index(line, ")");
      eq = index(line, "=");
      if (match(line, /^[[:space:]]*MD5[[:space:]]*\(/) && lp>0 && rp>lp && eq>rp) {
        path = substr(line, lp+1, rp - lp - 1);
        hash = substr(line, eq+1);
        gsub(/^[[:space:]]+/, "", hash);
        gsub(/[[:space:]]+$/, "", hash);
      } else {
        # Legacy: first two whitespace-separated fields
        split(line, a, /[ \t]+/);
        hash = a[1]; path = a[2];
      }

      # Normalize hash to lowercase
      hash = tolower(hash);

      # Validate hex length: 32 (md5) or 64 (sha256)
      if ((length(hash)==32 || length(hash)==64) && length(path)>0) {
        cksum_by_rel[path]=hash
        # Also map by basename as a fallback
        n=split(path, parts, "/"); base=parts[n]
        if (!(base in cksum_by_base)) cksum_by_base[base]=hash
      }
      next
    }

    # Phase 2: Process pbtxt file
    {
      line=$0
      if (index(line, "model_path:") && index(line, local_prefix)) {
        # Extract the relative path after the prefix
        # Example: model_path: "local:///mlperf_models/foo/bar.bin"
        start=index(line, local_prefix) + length(local_prefix)
        rel=substr(line, start)
        # rel currently includes trailing quote and maybe more; strip after next quote
        quote_pos=index(rel, "\"")
        if (quote_pos > 0) rel=substr(rel, 1, quote_pos-1)

        # Determine remote path (generic): always TARGET_URL + rel
        remote=TARGET_URL rel

        # Rebuild the model_path line with the remote URL
        prefix=substr(line, 1, index(line, "\"")-1)
        print prefix "\"" remote "\""
        pending_rel=rel
        changed=1
        next
      }

      # If the previous line changed a model_path, override the model_checksum that follows
      if (pending_rel!="" && index(line, "model_checksum:") ) {
        rel=pending_rel
        # Choose checksum: exact relative path first, then basename
        hash=""
        if (rel in cksum_by_rel) {
          hash=cksum_by_rel[rel]
        } else {
          n=split(rel, parts, "/"); base=parts[n]
          if (base in cksum_by_base) hash=cksum_by_base[base]
        }
        indent=substr(line, 1, index(line, "m")-1)
        print indent "model_checksum: \"" hash "\""
        pending_rel=""
        changed=1
        next
      }

      # Default: print line as-is
      print line
    }

    END {
    }
  ' "$CHECKSUM_FILE" "$f" > "$tmp"

  # Detect if file changed by comparing
  if ! cmp -s "$f" "$tmp"; then
    changes_in_file=1
    mv "$tmp" "$f"
    echo "Updated: $f"
  else
    rm -f "$tmp"
  fi


done

