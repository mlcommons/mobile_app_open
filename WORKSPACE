workspace(name = "mlperf_app")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_protobuf",
    sha256 = "528927e398f4e290001886894dac17c5c6a2e5548f3fb68004cfb01af901b53a",
    strip_prefix = "protobuf-3.17.3",
    urls = ["https://github.com/google/protobuf/archive/v3.17.3.zip"],
)

http_archive(
    name = "org_tensorflow",
    sha256 = "d2948c066a0bc3f45cb8072def03c85f50af8a75606bbdff91715ef8c5f2a28c",
    strip_prefix = "tensorflow-2.8.0",
    urls = [
        "https://github.com/tensorflow/tensorflow/archive/v2.8.0.zip",
    ],
)

# Initialize tensorflow workspace.
load("@org_tensorflow//tensorflow:workspace3.bzl", "tf_workspace3")

tf_workspace3()

load("@org_tensorflow//tensorflow:workspace2.bzl", "tf_workspace2")

tf_workspace2()

# Android.
android_sdk_repository(
    name = "androidsdk",
    api_level = 30,
)

android_ndk_repository(
    name = "androidndk",
)

RULES_JVM_EXTERNAL_TAG = "2.10"

RULES_JVM_EXTERNAL_SHA = "1bbf2e48d07686707dd85357e9a94da775e1dbd7c464272b3664283c9c716d26"

http_archive(
    name = "rules_jvm_external",
    sha256 = RULES_JVM_EXTERNAL_SHA,
    strip_prefix = "rules_jvm_external-%s" % RULES_JVM_EXTERNAL_TAG,
    url = "https://github.com/bazelbuild/rules_jvm_external/archive/%s.zip" % RULES_JVM_EXTERNAL_TAG,
)

load("@rules_jvm_external//:defs.bzl", "maven_install")

maven_install(
    artifacts = [
        "androidx.annotation:annotation:aar:1.1.0",
        "androidx.appcompat:appcompat:aar:1.1.0",
        "androidx.constraintlayout:constraintlayout:aar:2.0.0-beta3",
        "androidx.core:core:aar:1.1.0",
        "androidx.preference:preference:aar:1.1.0",
        "androidx.fragment:fragment:aar:1.2.0-rc04",
        "com.google.android.material:material:aar:1.2.0-alpha03",
        "androidx.recyclerview:recyclerview:aar:1.1.0",
        "androidx.lifecycle:lifecycle-livedata:aar:2.1.0",
        "junit:junit:4.13",
        "com.google.inject:guice:4.2.2",
        "org.hamcrest:java-hamcrest:2.0.0.0",
        "commons-io:commons-io:2.6",
        "androidx.test.espresso:espresso-core:3.2.0",
        "androidx.test.espresso:espresso-idling-resource:3.2.0",
        "androidx.test:runner:1.2.0",
        "androidx.test:rules:1.2.0",
        "androidx.test.ext:junit:1.1.2-alpha03",
    ],
    repositories = [
        "https://dl.google.com/dl/android/maven2",
        "https://repo1.maven.org/maven2",
    ],
)

# Other dependencies.
# Make this a repository so it can be substituted with the real backend

# use prebuilt neuron delegate for this submission, we'll update source code github later
# Neuron Delegate
#http_archive(
#    name = "neuron_delegate",
#    sha256 = "e80490919c87338fea402285142a629ca5795fbe08d8b4b1e071eb454c7a537d",
#    strip_prefix = "tflite-neuron-delegate-86fc333bafe8e556c050fe8cc32acf1c49e65847",
#    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/86fc333bafe8e556c050fe8cc32acf1c49e65847.tar.gz"],
#)

new_local_repository(
    name = "samsungbackend",
    build_file = "samsung_backend/BUILD",
    path = "mobile_back_samsung",
)

http_archive(
    name = "org_mlperf_inference",
    build_file = "@//android/third_party:loadgen.BUILD",
    patch_cmds = ["python3 loadgen/version_generator.py loadgen/version_generated.cc loadgen"],
    sha256 = "f4c57a3f3cd71f2dac166a79ad760b824aafda7b91400889acff4a9c7dbdaf8e",
    strip_prefix = "inference-a77ac37d07145d9f3123465a8fd18f9ebbde5d6a",
    urls = [
        "https://github.com/mlcommons/inference/archive/a77ac37d07145d9f3123465a8fd18f9ebbde5d6a.tar.gz",
    ],
)

http_archive(
    name = "backend_dummy_api",
    sha256 = "e478562b240623e18ada3d5bdfd00c6d2f15faf929df0d63e4a85f1b00b0f235",
    strip_prefix = "mobile_app-bf8a793d6f726bd72082455c1625e30cd6ab992c",
    urls = [
        "https://github.com/mlperf/mobile_app/archive/bf8a793d6f726bd72082455c1625e30cd6ab992c.zip",
    ],
)

# Specify the minimum required bazel version.
load("@org_tensorflow//tensorflow:version_check.bzl", "check_bazel_version_at_least")

check_bazel_version_at_least("3.7.2")

# Dependencies for android instrument test.
ATS_TAG = "1edfdab3134a7f01b37afabd3eebfd2c5bb05151"

ATS_SHA256 = "dcd1ff76aef1a26329d77863972780c8fe1fc8ff625747342239f0489c2837ec"

http_archive(
    name = "android_test_support",
    sha256 = ATS_SHA256,
    strip_prefix = "android-test-%s" % ATS_TAG,
    urls = ["https://github.com/android/android-test/archive/%s.tar.gz" % ATS_TAG],
)

load("@android_test_support//:repo.bzl", "android_test_repositories")

android_test_repositories()

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "9f9eb6cdd25d7932cb939df24807c2d70772aad7a79f1357e25ced9d0d443cfd",
    strip_prefix = "rules_apple-0.19.0",
    urls = [
        "https://github.com/bazelbuild/rules_apple/archive/refs/tags/0.19.0.zip",
    ],
)

http_archive(
    name = "build_bazel_rules_swift",
    sha256 = "ef728d0d99276d62b2393c350f29f176a6f38a925f2d12c37c4ed64f6906c2f5",
    strip_prefix = "rules_swift-0.13.0",
    urls = [
        "https://github.com/bazelbuild/rules_swift/archive/refs/tags/0.13.0.zip",
    ],
)

http_archive(
    name = "build_bazel_apple_support",
    sha256 = "249be3d90bc4211928a5260c4bc5792a236c58d1b6183c0e30f58db8710fc952",
    strip_prefix = "apple_support-0.7.2",
    urls = [
        "https://github.com/bazelbuild/apple_support/archive/refs/tags/0.7.2.zip",
    ],
)

# This is required to pass SNPE SDK path from external environment to sources,
# without actually modifying files
load("//mobile_back_qti:variables.bzl", "snpe_version_loader")

snpe_version_loader(
    name = "snpe_version_loader",
    workspace_dir = __workspace_dir__,
)
