# Building MLPerf Open app with Samsung backend

To build the app, use the following command

```bash
bazel-4.2.1 build  --verbose_failures -c opt --cxxopt='--std=c++14' --host_cxxopt='--std=c++14' --fat_apk_cp
u=arm64-v8a   --//android/java/org/mlperf/inference:with_samsung="1"  //android/java/org/mlperf/inference:ml
perf_app
```
