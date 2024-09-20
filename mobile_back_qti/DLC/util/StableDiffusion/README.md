# Stable Diffusion

### This readme contains necessary steps to:
* Run AIMET quantization
* Convert generated onnx files to bin files
* To generate all the artifacts needed for stable diffusion inference on Qualcomm Soc


### Platform requirements

* Machine running Ubuntu 20.04 at least
* AIMET PRO version 1.29.0 <code><b>(make script will automatically be installing it)</b></code>
* Docker version 20.10.24
* Machine enabled with Nvidia Tesla A100 or Tesla V100 (32GB at least)
* NVIDIA driver version equivalent to 525.60.13


### Steps to execute

`Please follow below steps in the mentioned order and run them as root to avoid permission issues`

#### Prerequisites
* Clone the mobile_app_open repository

* Install Qualcomm Package manager on the linux machine

```shell
sudo dpkg -i ./QualcommPackageManager3.3.0.111.1.Linux-x86.deb
```

* Extract the SNPE SDK (from Requirements above) to mobile_app_open/mobile_back_qti

```shell
qpm-cli --extract ./qualcomm_neural_processing_sdk.2.25.0.240728.Linux-AnyCPU.qik
mkdir mobile_app_open/mobile_back_qti/qairt/
cp -rv /opt/qcom/aistack/qairt/2.25.0.240728 mobile_app_open/mobile_back_qti/qairt/
```

Once done,

* Clone the AIMET SD notebook repository inside 
  <code><b><path_to_mobile_app_open>/mobile_back_qti/DLC/util/StableDiffusion/AIMET</b></code>
  
* Create hugging face access token and paste it on <code><b>line 2 of aimet.py</b></code> script, inside <code><b>/mobile_back_qti/DLC/util/StableDiffusion/AIMET</b></code> folder. 
  Place holder provided in <code><b>aimet.py</b></code>.
  
* Inside AIMET directory run this make command
   ```shell
    sudo make aimet_calibration
   ```
* Once, the above make command completes successfully, move to 
  <code><b><path_to_mobile_app_open>/mobile_back_qti/DLC</b></code> or type
   ```shell
    cd ../../../
   ```
* After reaching <code><b><path to mobile_app_open>/mobile_back_qti/DLC</b></code> run this make command
   ```shell   
    sudo make stable_diffusion_qnn SNPE_SDK=<path_to_mobile_app_open>/mobile_back_qti/qairt/<sdk_version>
   ```
* After successful execution, all the artifacts needed to run stable diffusion inference on device will be located in 
  <code><b><path_to_mobile_app_open>/output/DLC/mlperf_models/stable_diffusion</b></code>
