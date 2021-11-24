
This file describes how to easily update contents of the [cpp](/cpp) folder.

# Content

* [Updating C++ code](#updating-C-code)

# Updating C++ code

When there are some changes in default tflite backend
in the [reference repo](https://github.com/mlcommons/mobile_app_open)
(=repo of the Android app),
these changes should be pulled into this repo.

There is a branch
`[upstream/android-cpp](https://github.com/mlcommons/mobile_app_flutter/tree/upstream/android-cpp)`.
This branch contains unchanged source code from the reneferce repo.

To sync with the Android app, copy its source from the reference repo
into `upstream/android-cpp` branch,
then create a new branch from current main branch state
and merge `upstream/android-cpp` into your new branch,
and fix everything that became broken after the merge.
