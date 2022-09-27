# Building MLPerf Open app with Samsung backend

## Samsung backend directory [mobile_app_open/mobile_back_samsung/samsung](https://github.com/mlcommons/mobile_app_open/tree/master/mobile_back_samsung/samsung) with latest libs can be found here.
[samsung](https://github.com/mlcommons/mobile_back_samsung/tree/samsung_backend_flutter_libs/samsung_libs/mobile_back_samsung/samsung)


## To build the app, use the following command
```bash
make WITH_SAMSUNG=1 WITH_PIXEL=1 WITH_TFLITE=1 docker/flutter/android/apk
```
