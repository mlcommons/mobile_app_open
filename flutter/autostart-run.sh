#!/bin/bash

print_usage() {
cat << 'PRINT_HELP'
usage: ./autostart-run.sh [-h] [-u] [-s] [-d] [-l] [-o]
This script installs MLPerf app to a device, automatically starts the test,
and writes run logs into the specified directory.
When benchmark have finished, results json is printed on the screen and saved near log file.
optional arguments:
 -h Display usage
 -u Use official app UI
 -s Enable (s)ubmission mode to report per-usecase accuracy
 -d DEVICE_ID         Choose the device if several are connected (Default: No Default)
                      Run `flutter devices` to get ID of your device.
 -l OUTPUT_LOG_PATH   Specify where to save output logs          (Default: 'output/autostart_logs')
 -o OUTPUT_LOG_NAME   Specify what to name output logs           (Default: 'mlperf_<timestamp>')
PRINT_HELP
}

resultsStringMark=resultsStringMark
terminalStringMark=terminalStringMark
DEVICE_ID=""
official_build=false
submission_mode=false
OUTPUT_LOG_PATH="output/autostart_logs"
OUTPUT_LOG_NAME="mlperf_$(date +%Y-%m-%d-T%H-%M-%S)"

while getopts "h?usd:l:o:" flag
do
    case "$flag" in
        h)  print_usage;
            exit 0;;
        u)  official_build=true ;;
        s)  submission_mode=true ;;
        d)  DEVICE_ID=$OPTARG ;;
        l)  OUTPUT_LOG_PATH=$OPTARG ;;
        o)  OUTPUT_LOG_NAME=$OPTARG ;;
        # Unknown flag
        *)  print_usage;
            exit 1 ;;
    esac
done

echo "using parameters:"
echo "Official build set to \"$official_build\""
echo "Submission mode set to \"$submission_mode\""
echo "DEVICE_ID set to \"$DEVICE_ID\""
echo "OUTPUT_LOG_PATH set to \"$OUTPUT_LOG_PATH\""
echo "OUTPUT_LOG_NAME set to \"$OUTPUT_LOG_NAME\""

device_selector=""
if [[ "$DEVICE_ID" != "" ]]; then
    device_selector="-d $DEVICE_ID"
fi

mkdir -p "$OUTPUT_LOG_PATH"

echo "building and running the app... (this will take a while, see log file for intermediate results)"
flutter run \
    --dart-define=autostart=true \
    --dart-define=OFFICIAL_BUILD=$official_build \
    --dart-define=submission=$submission_mode \
    --dart-define=resultsStringMark=resultsStringMark \
    --dart-define=terminalStringMark=terminalStringMark \
    $device_selector \
    > "$OUTPUT_LOG_PATH/$OUTPUT_LOG_NAME" \
    || (echo "Failed to run app, see log file for details" && exit 1)

echo "done running"

results_path="$OUTPUT_LOG_PATH/$OUTPUT_LOG_NAME.json"

awk '/flutter: resultsStringMark/{flag=1; next}/flutter: terminalStringMark/{flag=0} flag' \
"$OUTPUT_LOG_PATH/$OUTPUT_LOG_NAME" | sed 's/flutter: //' >"$results_path"

echo "Results JSON path: \"$results_path\""
echo "---------------------------[ Results ]---------------------------"
cat "$results_path"
echo
