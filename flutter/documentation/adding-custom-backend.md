
This file describes how to add support for your backend.

# Contents

* [Implementing backend interface](#implementing-backend-interface)
* [Adding backend to list of backends](#adding-backend-to-list-of-backends)
* [Building the app with backends](#building-the-app-with-backends)

# Implementing backend interface

The following steps are required to add your backend:

1. Create a mobile_back_[vendor] top level directory that builds your backend using android/cpp/c/backend_c.h
2. Implement all the C API backend interfaces defined in android/cpp/c/backend_c.h
3. Add a bazel BUILD file to create lib[vendor]backend.so shared library

You can look at the [default TFLite backend implementation](../../mobile_back_tflite) for a reference.

# Adding backend to list of backends

1. Modify file [backends_list.in](../lib/backend/backends_list.in).
Add a line with your backend tag.
```dart
const _backendsList = [
    VENDOR_TAG
    TFLITE_TAG
];
```

**The order is important! The TFLITE_TAG must always be last and the probe order is first to last.**
2. Modify [makefile](../Makefile).
Make a copy of if-condition for `ENABLE_BACKEND_EXAMPLE` and change variable names and file name.
Add line for your backend to `set-supported-backends` make target.

Add build instructions for your backend
See the commented blocks in Makefile for backend/example-windows or backend/example-android.
Copy and modify the block as needed for your new backend changing "example" to the vendor name (e.g. intel)

# Building the app with backends

If you want to make a build with support for custom backends,
set corresponding environment variables and run full build,
or run `make set-supported-backends` directly.

