#!/bin/bash
# NPU binary generation script for Llama-3.1 only.

model_name="llama3.1"
echo "Running NPU bin generation for: $model_name"

chmod_success="chmod_success.stamp"

if [ ! -f "$chmod_success" ]; then
    chmod 777 -R *
    if [ $? -eq 0 ]; then
        echo "Permission set to 777 for current folder."
        touch "$chmod_success"
    else
        echo "Error setting permissions."
        exit 1
    fi
else
    echo "Chmod already executed"
fi

echo "Llama3.1"

step1_adascale_success="step1_adascale_success.stamp"
if [ ! -f "$step1_adascale_success" ]; then

    cp "run_example_1_adascale.py" "Step-1/"

    cd "Step-1"
    python run_example_1_adascale.py "$model_name"

    if [ $? -eq 0 ]; then
        echo "step1 adascale notebook python script executed successfully"
        cd "../"
        touch "$step1_adascale_success"
    else
        echo "Error running step1 adascale notebook python script."
        cd "../"
        exit 1
    fi
else
    echo "Step 1 Adascale Notebook already executed"
fi

step1_success="step1_success.stamp"
if [ ! -f "$step1_success" ]; then

    cp "run_example_1.py" "Step-1/"

    cd "Step-1"
    python run_example_1.py "$model_name"

    if [ $? -eq 0 ]; then
        echo "step1 notebook python script executed successfully"
        cd "../"
        touch "$step1_success"
    else
        echo "Error running step1 notebook python script."
        cd "../"
        exit 1
    fi
else
    echo "Step 1 Notebook already executed"
fi

step2_success="step2_success.stamp"
if [ ! -f "$step2_success" ]; then

    cp "run_example_2.py" "Step-2/host_linux/"

    cd "Step-2/host_linux/"
    python run_example_2.py "$model_name"

    if [ $? -eq 0 ]; then
        echo "Step2 notebook python script executed successfully"
        cd "../../"
        touch "$step2_success"
    else
        echo "Error running step2 notebook python script."
        cd "../../"
        exit 1
    fi
else
    echo "Step 2 Notebook already executed"
fi

copy_of_bins_success="copy_of_bins_success.stamp"
if [ ! -f "$copy_of_bins_success" ]; then
    cp -r "Step-2/host_linux/assets/ar128_ar32_cl512_cl1024_cl2048_cl3072_cl4096" "./"
    if [ $? -eq 0 ]; then
        echo "Bins file copied successfully to current folder"
        touch "$copy_of_bins_success"
    else
        echo "Error copying Bins."
        exit 1
    fi
else
    echo "Bins are already copied to current folder successfully"
fi

renaming_file_success="renaming_success.stamp"
if [ ! -f "$renaming_file_success" ]; then
    parent_dir="ar128_ar32_cl512_cl1024_cl2048_cl3072_cl4096"
    cd "$parent_dir" || exit
    pwd

    for file in `ls .`; do
        echo "$file"
        num=$(echo "$file" | cut -c 67)
        new_name="llama3_npu_${num}_of_6.bin"
        mv "$file" "$new_name"
        echo "Renamed $file to $new_name"
    done
    if [ $? -eq 0 ]; then
        echo "Bins file renamed successfully"
    else
        echo "Error in renaming the bins"
        cd ../
        exit 1
    fi
    cd ../
    touch "$renaming_file_success"
else
    echo "Bins are already renamed"
fi

copy_htp_backend_extension_success="copy_htp_BE_success.stamp"
if [ ! -f "$copy_htp_backend_extension_success" ]; then
    echo '{ "devices": [ { "soc_id": 660, "dsp_arch": "v81", "cores": [ { "core_id": 0, "perf_profile": "burst", "rpc_control_latency": 100 } ], "pd_session": "unsigned" } ], "context": { "weight_sharing_enabled": true, "extended_udma": true }, "memory": { "mem_type": "shared_buffer" }, "groupContext": { "share_resources": false } }' > htpBackendExtConfig.json
    cp "htpBackendExtConfig.json" "ar128_ar32_cl512_cl1024_cl2048_cl3072_cl4096/"
    if [ $? -eq 0 ]; then
        echo "Backend Extension is copied"
        touch "$copy_htp_backend_extension_success"
    else
        echo "Error in copying backend extension"
        exit 1
    fi
    rm htpBackendExtConfig.json
fi

rename_to_mlperf_models_success="rename_to_mlperf_models_success.stamp"
if [ ! -f "$rename_to_mlperf_models_success" ]; then
    mkdir -p "mlperf_models/llm"
    cp -r "ar128_ar32_cl512_cl1024_cl2048_cl3072_cl4096/." "mlperf_models/llm/"
    if [ $? -eq 0 ]; then
        echo "Contents copied to mlperf_models/llm/"
        touch "$rename_to_mlperf_models_success"
    else
        echo "Error copying contents to mlperf_models/llm/"
        exit 1
    fi
fi

copy_genie_config_success="copy_genie_config_success.stamp"
if [ ! -f "$copy_genie_config_success" ]; then
    if [ ! -f "genie_config_npu.json" ]; then
        echo "Error: genie_config_npu.json not found in current directory."
        exit 1
    fi
    cp "genie_config_npu.json" "mlperf_models/llm/"
    if [ $? -eq 0 ]; then
        echo "genie_config_npu.json copied to mlperf_models/llm/"
        touch "$copy_genie_config_success"
    else
        echo "Error copying genie_config_npu.json"
        exit 1
    fi
fi
