workspace(name = "mlperf_app")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_protobuf",
    sha256 = "9748c0d90e54ea09e5e75fb7fac16edce15d2028d4356f32211cfa3c0e956564",
    strip_prefix = "protobuf-3.11.4",
    urls = ["https://github.com/google/protobuf/archive/v3.11.4.zip"],
)

http_archive(
    name = "org_tensorflow",
    patch_args = ["-p1"],
    patches = ["//third_party:tf_gpu_delegate_fix_from_tf_master.diff"],
    sha256 = "e3d0ee227cc19bd0fa34a4539c8a540b40f937e561b4580d4bbb7f0e31c6a713",
    strip_prefix = "tensorflow-2.5.0",
    urls = [
        "https://github.com/tensorflow/tensorflow/archive/v2.5.0.zip",
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
    api_level = 29,
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

# Neuron Delegate

http_archive(
    name = "neuron_delegate",
    sha256 = "365fb6b82948b02072567def378fc9c3d6edfe3898d1978025172d00101b18cc",
    strip_prefix = "tflite-neuron-delegate-56908661f2558953d8850cace4d06d049cf4efc3",
    urls = ["https://github.com/MediaTek-NeuroPilot/tflite-neuron-delegate/archive/56908661f2558953d8850cace4d06d049cf4efc3.tar.gz"],
)

new_local_repository(
    name = "samsungbackend",
    build_file = "samsung_backend/BUILD",
    path = "mobile_back_samsung",
)

http_archive(
    name = "org_mlperf_inference",
    build_file = "@//android/third_party:loadgen.BUILD",
    patch_cmds = ["python3 loadgen/version_generator.py loadgen/version_generated.cc loadgen"],
    sha256 = "900053d97d4165396c45ec13dd211ea3cb3306a6c496b0172f7093e9fc5ebc47",
    strip_prefix = "inference-dd9e6bf867869fad0b9fcade0f719e1536780299",
    urls = [
        "https://github.com/mlperf/inference/archive/dd9e6bf867869fad0b9fcade0f719e1536780299.tar.gz",
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
