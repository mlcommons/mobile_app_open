
This file describes how to add support for your backend.

# Contents

* [Adding backend to list of backends](#adding-backend-to-list-of-backends)
* [Building the app with backends](#building-the-app-with-backends)
* [Implementing backend interface](#implementing-backend-interface)

# Adding backend to list of backends

1. Modify file [backends_list.in](/lib/backend/backends_list.in).
Add a line with your backend tag.
```dart
const _backendsList = [
    VENDOR_TAG
    TFLITE_TAG
];
```
2. Modify [makefile](/Makefile).
Make a copy of if-condition for `ENABLE_BACKEND_EXAMPLE` and change variable names and file name.
Add line for your backend to `set-supported-backends` make target.
3. Make your backend accessible to application.
    * If you want to keep sources of your backend in the main repository,
  add a make target that builds your backend and copies it to `build/backends` directory.
    * If you can't build your backend from sources, then just copy binary of your backend to `build/backends` directory.

# Building the app with backends

If you want to make a build with support for custom backends,
set corresponding environment variables and run full build,
or run `make set-supported-backends` directly.

# Implementing backend interface

Unfortunately, right now we don't have a separate documentation on how to implement a backend.

You can find declarations of C functions that you need to implement in [cpp/c](/cpp/c) directory.

You can look at the [default TFLite backend implementation](/cpp/backend_tflite) for a reference.
