#!/bin/bash
#==============================================================================
# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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
#==============================================================================

#------------------------------
# [reinstall.sh]
# This script performs autonomous installation, execution and logging of
# the MLPerf app
#------------------------------

# Run script with bash
if [ ! "$BASH_VERSION" ]; then
    exec /bin/bash "$0" "$@"
fi

unset MLPERF_PID
unset OUTPUT_LOG_NAME

# Constants
ANDROID_HOST=localhost
OUTPUT_LOG_PATH=output_logs
MLPERF_APK_PATH=output/mlperf_app.apk
MLPERF_PKG_NAME=org.mlperf.inference

TERMINAL_STRING='MLPerf Inference Phase Finished'\|'Fatal signal'
DEVICE_RESULTS_PATH=/storage/emulated/0/Android/data/org.mlperf.inference/files/mlperf/results.json

print_usage() {
cat << 'PRINT_HELP'
usage: ./reinstall.sh [-h] [-a] [-s] [-m] [-d] [-H] [-l] [-o]

This script clean installs, starts, and optionally automates running inference
on the MLPerf APK.

optional arguments:
 -h Display usage
 -a Enable (a)utostart mode to avoid Android UI interaction
 -s Enable (s)ubmission mode to report per-usecase accuracy
 -m MLPERF_APK_PATH   Specify the path to the MLPerf apk to install (Default: 'output/mlperf_app.apk')
 -d ANDROID_SERIAL    Specify target device's adb serial            (Default: No Default)
 -H ANDROID_HOST      Specify target device's host machine          (Default: 'localhost')
 -l OUTPUT_LOG_PATH   Specify where to save output logs             (Default: 'output_logs')
 -o OUTPUT_LOG_NAME   Specify what to name output logs              (Default: 'mlperf_<timestamp>')
PRINT_HELP
}

# Parse arguments
autostart=false
submission_mode=false

while getopts "h?asm:d:H:l:o:" flag
do
    case "$flag" in
        # Help
        h)  print_usage;
            exit 0;;
        a)  autostart=true ;;
        s)  submission_mode=true ;;
        m)  mlperf_apk_path=$OPTARG
            MLPERF_APK_PATH=`readlink -f $mlperf_apk_path`
            # Check file path exists and is accessible
            stat $MLPERF_APK_PATH > /dev/null 2>&1
            if [ ! $? -eq 0 ]; then
                echo "[ERROR] Could not stat \"$MLPERF_APK_PATH\""
                exit 1
            fi
            # Check file is an APK
            extension=$(basename $MLPERF_APK_PATH | tail -c 5)
            if [ "$extension" == ".apk" ]; then
                echo "[ERROR] Expected .apk file"
                exit 1
            fi
            echo "Using the following MLPerf APK: \"$MLPERF_APK_PATH\"" ;;
        d)  ANDROID_SERIAL=$OPTARG
            echo "ANDROID_SERIAL set to \"$ANDROID_SERIAL\"" ;;
        H)  ANDROID_HOST=$OPTARG
            echo "ANDROID_HOST set to \"$ANDROID_HOST\"" ;;
        l)  OUTPUT_LOG_PATH=$OPTARG ;;
        o)  OUTPUT_LOG_NAME=$OPTARG ;;
        # Unknown flag
        *)  print_usage;
            exit 1 ;;
    esac
done

# Sanity check on required env variables
echo
if [ ! -z "$ANDROID_SERIAL" ] && [ ! -z "$ANDROID_HOST" ]
then
    DEVICE_SPECIFIER="-H $ANDROID_HOST -s $ANDROID_SERIAL"
    echo "--------[ Target Device ]--------"
    echo "  Device Host:    $ANDROID_HOST"
    echo "  Device Serial:  $ANDROID_SERIAL"
    echo

    printf "Verifying connection to device... "
    adb $DEVICE_SPECIFIER shell ls > /dev/null 2>&1
    if [ ! $? -eq 0 ]; then
        echo "Failed"
        echo "[ERROR] The specified target is not accessible"
        exit 1
    fi
    echo "Success"
elif [ -z "$ANDROID_SERIAL" ]; then
    echo "ANDROID_SERIAL not set, please set this using -s argument"
    exit 1
else
    echo "ANDROID_HOST not set, please set this using -H argument"
    exit 1
fi


# Sanity check on log path
if [ ! -d $OUTPUT_LOG_PATH ]; then
    echo "Log directory does not exist: $OUTPUT_LOG_PATH"
    printf "Attempting to create the log directory... "
    mkdir -p "$OUTPUT_LOG_PATH"

    if [ ! -d $OUTPUT_LOG_PATH ]; then
        echo "Failed"
        exit 1
    fi

    echo "Success"
fi

# Wake up device
adb $DEVICE_SPECIFIER shell input keyevent KEYCODE_WAKEUP
adb $DEVICE_SPECIFIER shell input keyevent KEYCODE_MENU


# Uninstall and reinstall the APK
echo  "---------------------------------------------------"
echo  "Uninstalling and reinstalling MLPerf APK..."
echo  "---------------------------------------------------"
adb $DEVICE_SPECIFIER shell pm uninstall --user 0 $MLPERF_PKG_NAME > /dev/null 2>&1
adb $DEVICE_SPECIFIER install $MLPERF_APK_PATH


# Start the app on device
printf "\n---------------------------------------------------\n"
echo   "Starting up MLPerf..."
echo   "---------------------------------------------------"
# Grant permissions
adb $DEVICE_SPECIFIER shell pm grant $MLPERF_PKG_NAME android.permission.WRITE_EXTERNAL_STORAGE
adb $DEVICE_SPECIFIER shell pm grant $MLPERF_PKG_NAME android.permission.READ_PHONE_STATE
adb $DEVICE_SPECIFIER shell pm grant $MLPERF_PKG_NAME android.permission.READ_EXTERNAL_STORAGE
echo "Autostart:         $autostart"
echo "Submission Mode:   $submission_mode"
echo

adb $DEVICE_SPECIFIER shell am start \
    -n $MLPERF_PKG_NAME/.activities.CalculatingActivity \
    --ez "AUTO_START" $autostart \
    --ez "submission_mode" $submission_mode


# Logging and capture
if [ -z $OUTPUT_LOG_NAME ]
then
    TIMESTAMP=$(date +"%Y%m%d%H%M%S")
    LOG_PATH="${OUTPUT_LOG_PATH}/mlperf_${TIMESTAMP}_logcat.log"
    RESULTS_PATH="${OUTPUT_LOG_PATH}/mlperf_${TIMESTAMP}_results.json"
else
    LOG_PATH="${OUTPUT_LOG_PATH}/${OUTPUT_LOG_NAME}_logcat.log"
    RESULTS_PATH="${OUTPUT_LOG_PATH}/${OUTPUT_LOG_NAME}_results.json"
fi

printf "Attempting to fetch MLPerf App pid"
MLPERF_PID=$(adb $DEVICE_SPECIFIER shell "ps | sed -n 's/^[^ ]* *\([0-9]*\).* ${MLPERF_PKG_NAME}$/\1/p'")
timeout=5
tick=0.2
while [ "$MLPERF_PID" == "" ]
do
    printf "." && sleep $tick
    timeout=$(echo "$timeout - $tick" | bc)
    if [ "$timeout" == "0" ]; then
        printf "\n\n[ERROR] MLPerf ping timed out. PID of MLPerf App not detected on device!\n"
        exit 1
    fi

    MLPERF_PID=$(adb $DEVICE_SPECIFIER shell "ps | sed -n 's/^[^ ]* *\([0-9]*\).* ${MLPERF_PKG_NAME}$/\1/p'")
done
echo " Success!"

adb $DEVICE_SPECIFIER logcat -b all -c
adb $DEVICE_SPECIFIER logcat --pid=$MLPERF_PID | tee "$LOG_PATH" &
LOGCAT_PID=$!

echo "Logcat started for MLPerf App (pid=${MLPERF_PID})! Logging..."
( tail -f -n0 "$LOG_PATH" & ) | grep -q -E "$TERMINAL_STRING"
kill $LOGCAT_PID

printf "\nPulling results.json... "
adb $DEVICE_SPECIFIER pull "$DEVICE_RESULTS_PATH" "$RESULTS_PATH" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "Success!"
else
    echo "Failed!"
    exit 1
fi

# Display results
echo "Results JSON path: \"$RESULTS_PATH\""
echo "---------------------------[ Results ]---------------------------"
cat "$RESULTS_PATH" && echo
