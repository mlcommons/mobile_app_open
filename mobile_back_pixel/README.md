# Mobile backend tflite

Build so-library for selected architecture:

```bash
bazel build -c opt \
    --cxxopt='--std=c++14' \
    --host_cxxopt='--std=c++14' \
    --host_cxxopt='-Wno-deprecated-declarations' \
    --host_cxxopt='-Wno-class-memaccess' \
    --cxxopt='-Wno-deprecated-declarations' \
    --cxxopt='-Wno-unknown-attributes' \
    --fat_apk_cpu={x86_64|arm64-v8a|armeabi-v7a} \
    //cpp/backend_tflite:libtflitebackend.so
```
