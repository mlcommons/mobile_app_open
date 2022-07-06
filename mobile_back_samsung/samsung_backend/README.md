# Building MLPerf Open app with Samsung backend

### Copy required .so libs to the following directory 
mobile_app_open/mobile_back_samsung/samsung_backend/lib

### Link to required backend .so libs
https://github.com/mlcommons/mobile_back_samsung/tree/merge-submission-v2.0-into-master_samsung-backend/mobile_back_samsung/samsung_backend/lib

### To build the app using docker, use the following command 
```bash
make WITH_SAMSUNG=1 android/app
```

### OR, use build with Bazel directly 
```bash
bazel-4.2.1 build  --verbose_failures -c opt --cxxopt='--std=c++14' --host_cxxopt='--std=c++14' --fat_apk_cp
u=arm64-v8a   --//android/java/org/mlperf/inference:with_samsung="1"  //android/java/org/mlperf/inference:ml
perf_app
```

