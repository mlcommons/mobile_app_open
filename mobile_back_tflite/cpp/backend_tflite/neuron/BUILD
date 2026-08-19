load(
    "@litert//tflite:build_def.bzl",
    "tflite_copts",
    "tflite_jni_binary",
)
load("//flutter/cpp/proto:pbtxt2header.bzl", "pbtxt2header")

pbtxt2header(
    name = "tflite_settings",
    srcs = [
        "backend_settings/tflite_settings_mtk.pbtxt",
        "backend_settings/tflite_settings_mtk_mt6989.pbtxt",
        "backend_settings/tflite_settings_mtk_mt6991.pbtxt",
        "backend_settings/tflite_settings_mtk_mt6993.pbtxt",
    ],
)

# MediaTek's neuron_delegate.h includes tensorflow/lite/c/common.h, but this
# app links LiteRT. Depending on @org_tensorflow//tensorflow/lite/c:common to
# satisfy that include puts a second copy of the TF Lite C API in the .so and
# the link fails on duplicate symbols; patching the archive in place instead
# breaks its own targets, which are built against TensorFlow throughout.
#
# So take a copy of just that header with the include repointed at LiteRT's
# identical one. Quoted includes search the includer's directory first, so this
# shadows the vendor header for this package only and the archive is untouched.
genrule(
    name = "neuron_delegate_h",
    srcs = ["@neuron_delegate//neuron:neuron_delegate.h"],
    outs = ["neuron_delegate.h"],
    cmd = "sed -e 's|tensorflow/lite/c/common.h|tflite/c/common.h|' $< > $@",
)

cc_library(
    name = "neuron_delegate",
    srcs = select({
        "@org_tensorflow//tensorflow:android": [
            "@neuron_delegate//neuron/java:libtensorflowlite_neuron_jni.so",
        ],
    }),
    hdrs = select({
        "@org_tensorflow//tensorflow:android": [
            "@neuron_delegate//neuron:APUWareUtilsApi.h",
            ":neuron_delegate.h",
        ],
    }),
    deps = select({
        "@org_tensorflow//tensorflow:android": [
            "@litert//tflite/c:common",
        ],
    }),
)

cc_library(
    name = "tflite_neuron_c",
    srcs = [
        "neuron_backend.cc",
        "neuron_memory.cc",
        "//mobile_back_tflite/cpp/backend_tflite:llm_pipeline.cc",
        "//mobile_back_tflite/cpp/backend_tflite:sd_utils.cc",
        "//mobile_back_tflite/cpp/backend_tflite:single_model_pipeline.cc",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_invoker.cc",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_pipeline.cc",
        "//mobile_back_tflite/cpp/backend_tflite:tflite_c.cc",
    ],
    hdrs = [
        "APUWareUtilsLib.h",
        "NeuronAdapter.h",
        "NeuronAdapterShim.h",
        "neuron_backend.h",
        "neuron_builder.h",
        "neuron_memory.h",
        "neuron_utils.h",
        "tflite_settings_mtk.h",
        "//mobile_back_tflite/cpp/backend_tflite:llm_pipeline.h",
        "//mobile_back_tflite/cpp/backend_tflite:pipeline.h",
        "//mobile_back_tflite/cpp/backend_tflite:sd_utils.h",
        "//mobile_back_tflite/cpp/backend_tflite:single_model_pipeline.h",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_invoker.h",
        "//mobile_back_tflite/cpp/backend_tflite:stable_diffusion_pipeline.h",
        "//mobile_back_tflite/cpp/backend_tflite:tflite_settings_android.h",
        "//mobile_back_tflite/cpp/backend_tflite:tflite_settings_apple.h",
        "//mobile_back_tflite/cpp/backend_tflite:tflite_settings_windows.h",
        "//mobile_back_tflite/cpp/backend_tflite:thread_pool.h",
        "//mobile_back_tflite/cpp/backend_tflite:utils.h",
    ],
    copts = tflite_copts() + [
        "-Iexternal/neuron_delegate",
    ] + select({
        "//flutter/android/commonlibs:use_asan": [
            "-fsanitize=address",
            "-g",
            "-O1",
            "-fno-omit-frame-pointer",
        ],
        "//conditions:default": [],
        # fpermissive required due to bug in LiteRT (as of version 2.1.2)
    }) + [
        # Same as //mobile_back_tflite/cpp/backend_tflite:tflite_c: this target
        # shares its sources, which call TfLiteInterpreterOptionsAddDelegate
        # with a TfLiteDelegate rather than the opaque type.
        "-UTFLITE_USE_OPAQUE_DELEGATE",
        "-fpermissive",
    ],
    local_defines = ["MTK_TFLITE_NEURON_BACKEND"],
    deps = [
        ":tflite_settings",
        "//flutter/cpp:utils",
        "//flutter/cpp/c:headers",
        "//mobile_back_tflite/cpp/backend_tflite:embedding_utils",
        "//mobile_back_tflite/cpp/backend_tflite:tflite_settings",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
        "@litert//tflite/c:c_api",
        "@litert//tflite/c:common",
        "@litert//tflite/experimental/genai:genai_ops",
        "@litert//tflite/kernels:builtin_ops",
        # This target compiles tflite_c.cc and llm_pipeline.cc, which use the
        # LiteRT compiled-model API, so it needs the same deps as
        # //mobile_back_tflite/cpp/backend_tflite:tflite_c.
        "@litert//litert/cc:litert_compiled_model",
        "@litert//litert/cc:litert_environment",
        "@litert//litert/cc:litert_tensor_buffer",
        "@litert//litert/c:litert_environment",
        "@litert//litert/c:litert_environment_options",
        "@litert//litert/c:litert_event",
        "@litert//litert/c:litert_tensor_buffer",
        "@litert//litert/c:litert_opaque_options",
        "@litert//litert/c/options:litert_gpu_options",
        "@litert//litert/c/internal:litert_accelerator_registration",
        "@litert//litert/c/internal:litert_tensor_buffer_registry",
        "@litert//litert/c/internal:litert_external_litert_buffer_context",
    ] + select({
        "@org_tensorflow//tensorflow:android": [
            ":neuron_delegate",
            "@litert//tflite/delegates/gpu:delegate",
        ],
        "//conditions:default": [],
    }),
    alwayslink = 1,
)

tflite_jni_binary(
    name = "libtfliteneuronbackend.so",
    linkscript = "//flutter/cpp/c:version_script.lds",
    deps = [
        ":tflite_neuron_c",
    ],
)

aar_import(
    name = "tflite_neuron_aar",
    aar = "neuron/tensorflow-lite-neuron.aar",
)
