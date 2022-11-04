# Building MLPerf Open app with Samsung backend

<!-- markdown-link-check-disable-next-line -->
1. Update the "lib" folder at 'mobile_app_open/mobile_back_samsung/samsung/lib' by copying it from [here](https://github.com/mlcommons/mobile_back_samsung/tree/submission_v2.1_samsung_backend/samsung_libs/mobile_back_samsung/samsung). Here are the .so libs hashes:
```
5aceeaf47af8c3c1847906a12a7b5de70260e5cbd36a70ab2d163c08682399eb  libsamsungbackend.so
2104ff038a520180ac4eb6ca5797fa4bd93f1a50943a4412de9bb783947b3194  libsbe1200_core.so
9333a90f065f65610e0ddc788003b33b55c9940f73e665b3d6a0282de11c046c  libsbe2100_core.so
959e772790516649eb5454fd17c4219e3778e35af620061e5c62c6c4a88e1d67  libsbe2200_core.so
d5a475d949018179c8e6aa18a5e524c79197a55e07092d5a63e04d5da10d281c  libsbe2300_core.so
```
2. Change directory to the main path (mobile_app_open)
3. Run the build command

```bash
make WITH_SAMSUNG=1 OFFICIAL_BUILD=false FLUTTER_BUILD_NUMBER=0 docker/flutter/android/release
```
