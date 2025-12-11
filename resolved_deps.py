resolved = [
     {
          "original_rule_class": "local_repository",
          "original_attributes": {
               "name": "bazel_tools",
               "path": "/var/tmp/_bazel_anh/install/28992672a83e268a373e6b60cf8c5ee7/embedded_tools"
          },
          "native": "local_repository(name = \"bazel_tools\", path = __embedded_dir__ + \"/\" + \"embedded_tools\")"
     },
     {
          "original_rule_class": "//:platform.bzl%tf_patch_finder",
          "definition_information": "Repository tf_patch_finder instantiated at:\n  /Users/anh/dev/mlcommons/mobile_app_open/WORKSPACE:24:16: in <toplevel>\nRepository rule tf_patch_finder defined at:\n  /Users/anh/dev/mlcommons/mobile_app_open/platform.bzl:16:34: in <toplevel>\n",
          "original_attributes": {
               "name": "tf_patch_finder",
               "workspace_dir": "/Users/anh/dev/mlcommons/mobile_app_open"
          },
          "repositories": [
               {
                    "rule_class": "//:platform.bzl%tf_patch_finder",
                    "attributes": {
                         "name": "tf_patch_finder",
                         "workspace_dir": "/Users/anh/dev/mlcommons/mobile_app_open"
                    },
                    "output_tree_hash": "cb60cc775083903286012fed258cb348af615362f12f73bcda540ef102906c5f"
               }
          ]
     },
     {
          "original_rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
          "definition_information": "Repository rules_python instantiated at:\n  /Users/anh/dev/mlcommons/mobile_app_open/WORKSPACE:15:13: in <toplevel>\nRepository rule http_archive defined at:\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/bazel_tools/tools/build_defs/repo/http.bzl:431:31: in <toplevel>\n",
          "original_attributes": {
               "name": "rules_python",
               "url": "https://github.com/bazelbuild/rules_python/releases/download/0.25.0/rules_python-0.25.0.tar.gz",
               "sha256": "5868e73107a8e85d8f323806e60cad7283f34b32163ea6ff1020cf27abef6036",
               "strip_prefix": "rules_python-0.25.0"
          },
          "repositories": [
               {
                    "rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
                    "attributes": {
                         "name": "rules_python",
                         "url": "https://github.com/bazelbuild/rules_python/releases/download/0.25.0/rules_python-0.25.0.tar.gz",
                         "sha256": "5868e73107a8e85d8f323806e60cad7283f34b32163ea6ff1020cf27abef6036",
                         "strip_prefix": "rules_python-0.25.0"
                    },
                    "output_tree_hash": "a381213e8454f4f62102d12f179d1d0773e1517ac873f2192c6758fcaaf6611b"
               }
          ]
     },
     {
          "original_rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
          "definition_information": "Repository org_tensorflow instantiated at:\n  /Users/anh/dev/mlcommons/mobile_app_open/WORKSPACE:51:13: in <toplevel>\nRepository rule http_archive defined at:\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/bazel_tools/tools/build_defs/repo/http.bzl:431:31: in <toplevel>\n",
          "original_attributes": {
               "name": "org_tensorflow",
               "urls": [
                    "https://github.com/tensorflow/tensorflow/archive/v2.14.0.tar.gz"
               ],
               "sha256": "ce357fd0728f0d1b0831d1653f475591662ec5bca736a94ff789e6b1944df19f",
               "strip_prefix": "tensorflow-2.14.0",
               "patches": [
                    "//:flutter/third_party/enable-png-in-tensorflow-lite-tools-evaluation.patch",
                    "//:flutter/third_party/png-with-number-of-channels-detected.patch",
                    "//:flutter/third_party/use_unsigned_char.patch",
                    "//:flutter/third_party/tensorflow-fix-file-opening-mode-for-Windows.patch",
                    "//:flutter/third_party/tf-eigen.patch",
                    "//patches:ndk_25_r14.diff"
               ],
               "patch_args": [
                    "-p1"
               ]
          },
          "repositories": [
               {
                    "rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
                    "attributes": {
                         "name": "org_tensorflow",
                         "urls": [
                              "https://github.com/tensorflow/tensorflow/archive/v2.14.0.tar.gz"
                         ],
                         "sha256": "ce357fd0728f0d1b0831d1653f475591662ec5bca736a94ff789e6b1944df19f",
                         "strip_prefix": "tensorflow-2.14.0",
                         "patches": [
                              "//:flutter/third_party/enable-png-in-tensorflow-lite-tools-evaluation.patch",
                              "//:flutter/third_party/png-with-number-of-channels-detected.patch",
                              "//:flutter/third_party/use_unsigned_char.patch",
                              "//:flutter/third_party/tensorflow-fix-file-opening-mode-for-Windows.patch",
                              "//:flutter/third_party/tf-eigen.patch",
                              "//patches:ndk_25_r14.diff"
                         ],
                         "patch_args": [
                              "-p1"
                         ]
                    },
                    "output_tree_hash": "720508d54a43d77789d8f4edac8691503373e0458a8b715695b1318dac736ffd"
               }
          ]
     },
     {
          "original_rule_class": "@@org_tensorflow//tensorflow/tools/toolchains/python:python_repo.bzl%python_repository",
          "definition_information": "Repository python_version_repo instantiated at:\n  /Users/anh/dev/mlcommons/mobile_app_open/WORKSPACE:78:18: in <toplevel>\nRepository rule python_repository defined at:\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/org_tensorflow/tensorflow/tools/toolchains/python/python_repo.bzl:29:36: in <toplevel>\n",
          "original_attributes": {
               "name": "python_version_repo"
          },
          "repositories": [
               {
                    "rule_class": "@@org_tensorflow//tensorflow/tools/toolchains/python:python_repo.bzl%python_repository",
                    "attributes": {
                         "name": "python_version_repo"
                    },
                    "output_tree_hash": "673a05cb61441837d24ae9eaafc47a5a57c0a31f3c8237e45c19ad0b5f0afb7b"
               }
          ]
     },
     {
          "original_rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
          "definition_information": "Repository bazel_skylib instantiated at:\n  /Users/anh/dev/mlcommons/mobile_app_open/WORKSPACE:6:13: in <toplevel>\nRepository rule http_archive defined at:\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/bazel_tools/tools/build_defs/repo/http.bzl:431:31: in <toplevel>\n",
          "original_attributes": {
               "name": "bazel_skylib",
               "urls": [
                    "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
                    "https://github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz"
               ],
               "sha256": "66ffd9315665bfaafc96b52278f57c7e2dd09f5ede279ea6d39b2be471e7e3aa"
          },
          "repositories": [
               {
                    "rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
                    "attributes": {
                         "name": "bazel_skylib",
                         "urls": [
                              "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz",
                              "https://github.com/bazelbuild/bazel-skylib/releases/download/1.4.2/bazel-skylib-1.4.2.tar.gz"
                         ],
                         "sha256": "66ffd9315665bfaafc96b52278f57c7e2dd09f5ede279ea6d39b2be471e7e3aa"
                    },
                    "output_tree_hash": "4ae8283d310bb776140e679603e7d708a91f4c25cd0af9cbdc53b57889c2e913"
               }
          ]
     },
     {
          "original_rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
          "definition_information": "Repository io_bazel_rules_closure instantiated at:\n  /Users/anh/dev/mlcommons/mobile_app_open/WORKSPACE:91:14: in <toplevel>\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/org_tensorflow/tensorflow/workspace3.bzl:8:17: in workspace\nRepository rule http_archive defined at:\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/bazel_tools/tools/build_defs/repo/http.bzl:431:31: in <toplevel>\n",
          "original_attributes": {
               "name": "io_bazel_rules_closure",
               "generator_name": "io_bazel_rules_closure",
               "generator_function": "workspace",
               "generator_location": None,
               "urls": [
                    "https://storage.googleapis.com/mirror.tensorflow.org/github.com/bazelbuild/rules_closure/archive/308b05b2419edb5c8ee0471b67a40403df940149.tar.gz",
                    "https://github.com/bazelbuild/rules_closure/archive/308b05b2419edb5c8ee0471b67a40403df940149.tar.gz"
               ],
               "sha256": "5b00383d08dd71f28503736db0500b6fb4dda47489ff5fc6bed42557c07c6ba9",
               "strip_prefix": "rules_closure-308b05b2419edb5c8ee0471b67a40403df940149"
          },
          "repositories": [
               {
                    "rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
                    "attributes": {
                         "name": "io_bazel_rules_closure",
                         "generator_name": "io_bazel_rules_closure",
                         "generator_function": "workspace",
                         "generator_location": None,
                         "urls": [
                              "https://storage.googleapis.com/mirror.tensorflow.org/github.com/bazelbuild/rules_closure/archive/308b05b2419edb5c8ee0471b67a40403df940149.tar.gz",
                              "https://github.com/bazelbuild/rules_closure/archive/308b05b2419edb5c8ee0471b67a40403df940149.tar.gz"
                         ],
                         "sha256": "5b00383d08dd71f28503736db0500b6fb4dda47489ff5fc6bed42557c07c6ba9",
                         "strip_prefix": "rules_closure-308b05b2419edb5c8ee0471b67a40403df940149"
                    },
                    "output_tree_hash": "a0a27ed42797fa7d222bdf985af72d4cdd666b6a0b23e48aa370bb99a677d58d"
               }
          ]
     },
     {
          "original_rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
          "definition_information": "Repository rules_jvm_external instantiated at:\n  /Users/anh/dev/mlcommons/mobile_app_open/WORKSPACE:91:14: in <toplevel>\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/org_tensorflow/tensorflow/workspace3.bzl:41:17: in workspace\nRepository rule http_archive defined at:\n  /private/var/tmp/_bazel_anh/30b0ae0ebfc82d789f6eeabcb52f979d/external/bazel_tools/tools/build_defs/repo/http.bzl:431:31: in <toplevel>\n",
          "original_attributes": {
               "name": "rules_jvm_external",
               "generator_name": "rules_jvm_external",
               "generator_function": "workspace",
               "generator_location": None,
               "url": "https://github.com/bazelbuild/rules_jvm_external/archive/4.3.zip",
               "sha256": "6274687f6fc5783b589f56a2f1ed60de3ce1f99bc4e8f9edef3de43bdf7c6e74",
               "strip_prefix": "rules_jvm_external-4.3"
          },
          "repositories": [
               {
                    "rule_class": "@@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
                    "attributes": {
                         "name": "rules_jvm_external",
                         "generator_name": "rules_jvm_external",
                         "generator_function": "workspace",
                         "generator_location": None,
                         "url": "https://github.com/bazelbuild/rules_jvm_external/archive/4.3.zip",
                         "sha256": "6274687f6fc5783b589f56a2f1ed60de3ce1f99bc4e8f9edef3de43bdf7c6e74",
                         "strip_prefix": "rules_jvm_external-4.3"
                    },
                    "output_tree_hash": "327a52f739c3b078117d9ded0b0630fea535c193cf6ff41764bd4b2eae469334"
               }
          ]
     }
]