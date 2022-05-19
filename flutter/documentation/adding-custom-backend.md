# Adding custom backend

This file describes how to add support for your backend into the Flutter app.

## Contents

* [Folder structure](#folder-structure)
* [Integrating your backend into the app](#integrating-your-backend-into-the-app)
* [Implementing backend interface](#implementing-backend-interface)

## Folder structure

Create a new folder for your backend, named `mobile_back_<vendor>`, where `<vendor>` is the name of your organization or name of your backend, if you have one.
Structure inside that your backend directory in not strictly regulated, you may keep it the way that is convenient to you. Just keep it organized.
See existing `mobile_back_*` folders for examples on how you you can structure your files.

Generally it's a good idea to keep all C++ files in a `cpp` folder.

Add a readme file explaining how to set up environment to build your backend and which commands to use.

Add `<vendor>_backend.mk` file defining variables for your backend, similar to such files for other backends. This file should have the following structure:

```make
ifeq (${WITH_<VENDOR>},1)
  $(info WITH_<VENDOR>=1)
  backend_<vendor>_android_files=${BAZEL_LINKS_PREFIX}bin/mobile_back_<vendor>/path/to/lib.so
  backend_<vendor>_android_target=//mobile_back_<vendor>/path/to:lib.so
  backend_<vendor>_filename=lib
endif
```

If your backend is targeted to a platform other than Android, change platform name in the names of variables.

Filename of the backend can technically be anything but the common convention is to use `lib<vendor>backend`.

It is advised that you use `bazel` as build system for your backend.
However, this is not a strict requirement.
Our Flutter app requires only final binaries, so any build system can theoretically be used.
Set `backend_<vendor>_android_target` to appropriate value for your build system.

## Integrating your backend into the app

You will need to do few things:

* Include your `<vendor>_backend.mk` into the root makefile.
* Add new line with your `<VENDOR>_TAG` into [list.in](../lib/backend/list.in).  
Note that order is important in this file. Backends are evaluated in the order they are defined, and the app never checks backends after TFLite.
Place your tag before TFLite tag.
* Add line to substitute this tag with the actual name of your backend lib into [flutter.mk](../flutter.mk) in the `flutter/backend-list` make target
* Add commands to build your backend when `WITH_<VENDOR>=1` make variable is supplied.
  Modify `flutter/android/libs` ([android.mk](../android/android.mk)) or `flutter/windows/libs` ([windows.mk](../windows/windows.mk)), depending on your platform:
  * If you use bazel as your build system, just add a line `${backend_<vendor>_android_target}` into appropriate position
  * If you use different build system, add a condition to only build your backend when `WITH_<VENDOR>=1` is defined.
  * Add a line `${backend_<vendor>_android_files}` into the copy command

## Implementing backend interface

Backend C API is defined in [flutter/cpp/c/backend_c.h](../../flutter/cpp/c/backend_c.h).  
Unfortunately, we don't have any documentation on details of how to implement it yet.  
You can look at the [reference TFLite backend implementation](../../mobile_back_tflite) for hints or create an issue if something is not clear.
