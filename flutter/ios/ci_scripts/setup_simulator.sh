set -euo pipefail

# DEVICE_NAME="iPhone 16 Pro"
# RUNTIME_SDK="iphonesimulator18.1"
# Allow overriding device type by ID; fallback to name for display purposes
DEVICE_TYPE_ID="${DEVICE_TYPE_ID:-}"
DEVICE_NAME="${DEVICE_NAME:-${DEVICE_TYPE_ID:-iPhone 16 Pro}}"

# Build the runtime ID (must use SimRuntime namespace)
RUNTIME_ID="com.apple.CoreSimulator.SimRuntime.iOS-$(echo "$RUNTIME_SDK" | sed -E 's/[^0-9]*([0-9]+)\.([0-9]+).*/\1-\2/')"

# Validate that the requested device type exists
if [ -n "${DEVICE_TYPE_ID}" ]; then
  xcrun simctl list devicetypes | grep -q "(${DEVICE_TYPE_ID})" || { echo "Device type ID '${DEVICE_TYPE_ID}' not found"; exit 1; }
else
  xcrun simctl list devicetypes | grep -q "^${DEVICE_NAME} " || { echo "Device type '${DEVICE_NAME}' not found"; exit 1; }
fi

# Validate that the requested runtime exists
xcrun simctl list runtimes | grep -q "${RUNTIME_ID}" || { echo "Runtime '${RUNTIME_ID}' not installed on this runner"; exit 1; }

# Try to locate an existing device of the given type and runtime, otherwise create it
UDID=$(xcrun simctl list devices "${RUNTIME_ID}" | awk -v dev="${DEVICE_NAME}" -F'[()]' '
  $0 ~ dev {
    cand=$(NF-1)
    gsub(/^[[:space:]]+|[[:space:]]+$/, "", cand)
    if (cand ~ /^[0-9A-Fa-f-]{36}$/) { print cand; exit }
  }')
if [ -z "${UDID:-}" ]; then
  TYPE_ARG="${DEVICE_TYPE_ID:-${DEVICE_NAME}}"
  UDID=$(xcrun simctl create "CI ${DEVICE_NAME}" "${TYPE_ARG}" "${RUNTIME_ID}")
  echo "Created ${DEVICE_NAME}: ${UDID}"
else
  echo "Reusing ${DEVICE_NAME}: ${UDID}"
fi

xcrun simctl boot "${UDID}" || true
xcrun simctl bootstatus "${UDID}" -b
defaults write com.apple.iopen -a Simulatorphonesimulator "CurrentDeviceUDID" "${UDID}"
