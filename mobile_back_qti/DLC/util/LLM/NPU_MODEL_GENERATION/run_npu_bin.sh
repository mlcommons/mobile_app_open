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

asset_dir_name="ar128_ar32_cl512_cl1024_cl2048_cl3072_cl4096"
generated_asset_dir="Step-2/host_linux/assets/${asset_dir_name}"

generate_target() {
    local target_name="$1"
    local platform_gen="$2"
    local soc_id="$3"
    local dsp_arch="$4"
    local target_platform="$5"
    local step2_stamp="step2_${target_name}_success.stamp"
    local copy_stamp="copy_of_bins_${target_name}.stamp"
    local package_stamp="package_${target_name}_success.stamp"
    local target_asset_copy="${asset_dir_name}_${target_name}"
    local output_dir="mlperf_models/llm_${target_name}"
    local file
    local required_file
    local index
    local bin_count

    echo "=============================================="
    echo "Generating Llama NPU binaries for ${target_name}"
    echo "TARGET_PLATFORM=${target_platform} PLATFORM_GEN=${platform_gen} soc_id=${soc_id} dsp_arch=${dsp_arch}"
    echo "=============================================="

    if [ -f "$package_stamp" ] && [ -d "$output_dir" ] \
        && [ "$(find "$output_dir" -maxdepth 1 -type f -name 'llama3_npu_*_of_6.bin' | wc -l)" -eq 6 ] \
        && [ -f "$output_dir/htpBackendExtConfig.json" ] \
        && [ -f "$output_dir/genie_config_npu.json" ]; then
        echo "${target_name} package already completed"
        return 0
    fi

    export TARGET_PLATFORM="$target_platform"
    export PLATFORM_GEN="$platform_gen"

    if [ ! -f "$step2_stamp" ] || [ ! -d "$generated_asset_dir" ]; then
        if [ -d "$generated_asset_dir" ]; then
            rm -rf -- "$generated_asset_dir" || return 1
        fi

        cp "run_example_2.py" "Step-2/host_linux/" || return 1

        if ! (
            find Step-2 -type f -exec grep -Il "$(printf '\r')" {} + 2>/dev/null \
                | xargs -r sed -i 's/\r$//'
            cd "Step-2/host_linux/" || exit 1
            python run_example_2.py "$model_name"
        ); then
            echo "Error running Step-2 for ${target_name}."
            return 1
        fi

        touch "$step2_stamp" || return 1
        echo "Step-2 completed for ${target_name}"
    else
        echo "Step-2 already completed for ${target_name}"
    fi

    if [ ! -d "$generated_asset_dir" ]; then
        echo "Error: generated asset directory not found: $generated_asset_dir"
        return 1
    fi

    if [ ! -f "$copy_stamp" ] || [ ! -d "$target_asset_copy" ]; then
        if [ -e "$target_asset_copy" ]; then
            rm -rf -- "$target_asset_copy" || return 1
        fi

        cp -r "$generated_asset_dir" "$target_asset_copy" || return 1

        index=1
        for file in "$target_asset_copy"/*; do
            [ -f "$file" ] || continue
            case "$file" in
                *.json) continue ;;
            esac

            mv -- "$file" "$target_asset_copy/llama3_npu_${index}_of_6.bin" || return 1
            echo "Renamed $file to llama3_npu_${index}_of_6.bin"
            index=$((index + 1))
        done

        bin_count=$((index - 1))
        if [ "$bin_count" -ne 6 ]; then
            echo "Error: expected 6 generated binaries for ${target_name}, found ${bin_count}."
            return 1
        fi

        touch "$copy_stamp" || return 1
        echo "Bins copied and renamed for ${target_name}"
    else
        echo "Bins are already copied and renamed for ${target_name}"
    fi

    mkdir -p "$output_dir" || return 1
    rm -f \
        "$output_dir"/llama3_npu_*_of_6.bin \
        "$output_dir"/htpBackendExtConfig.json \
        "$output_dir"/genie_config_npu.json || return 1

    cp -r "$target_asset_copy/." "$output_dir/" || return 1

    cat > "$output_dir/htpBackendExtConfig.json" <<EOF
{
    "devices": [
        {
            "soc_id": ${soc_id},
            "dsp_arch": "${dsp_arch}",
            "cores": [
                {
                    "core_id": 0,
                    "perf_profile": "burst",
                    "rpc_control_latency": 100
                }
            ],
            "pd_session": "unsigned"
        }
    ],
    "context": {
        "weight_sharing_enabled": true,
        "extended_udma": true
    },
    "memory": {
        "mem_type": "shared_buffer"
    },
    "groupContext": {
        "share_resources": false
    }
}
EOF

    cp -f "genie_config_npu.json" "$output_dir/" || return 1

    touch "$package_stamp" || return 1
    echo "Completed ${target_name}: ${output_dir}"
}
generate_target "sd8eliteG5" 3 88 v81 "Windows" || exit 1
generate_target "sd8eliteEG6" 5 103 v85 "Android" || exit 1
generate_target "sd8eliteG6" 5 103 v85 "Android" || exit 1


echo "All target packages generated successfully."
