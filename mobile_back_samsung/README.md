# Building MLPerf Open app with Samsung backend

<!-- markdown-link-check-disable-next-line -->
1. Update the "lib/internal" folder at 'mobile_app_open/mobile_back_samsung/samsung/lib/internal' by copying the libs from [here](https://github.com/mlcommons/mobile_back_samsung/tree/submission_v5.0_samsung_backend/samsung_libs)
2. Change directory to the main path (mobile_app_open)
3. Run the build command

```bash
make WITH_SAMSUNG=1 OFFICIAL_BUILD=false FLUTTER_BUILD_NUMBER=0 docker/flutter/android/release
```
