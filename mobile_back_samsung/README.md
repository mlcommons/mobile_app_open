# Building MLPerf Open app with Samsung backend

1. Update the "lib" folder at 'mobile_app_open/mobile_back_samsung/samsung/lib' by copying it from 
[here](https://github.com/mlcommons/mobile_back_samsung/tree/samsung_backend_flutter_libs/samsung_libs/mobile_back_samsung/samsung)

2. Change directory to the main path (mobile_app_open)

3. Run the build command
```bash
make WITH_SAMSUNG=1 docker/flutter/android/apk
```

