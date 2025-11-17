package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])

exports_files(["LICENSE"])

genrule(
    name = "mlperf_conf_h",
    srcs = ["mlperf.conf"],
    outs = ["mlperf_conf.h"],
    cmd_bash = r"""
      set -euo pipefail
      in="$(location mlperf.conf)"
      out="$@"

      # Start the C string
      printf 'const char* mlperf_conf =\n' > "$$out"

      # Read all lines, including a final line without newline
      while IFS= read -r line || [ -n "$$line" ]; do
        line_esc=$${line//\\/\\\\}
        line_esc=$${line_esc//\"/\\\"}
        printf '"%s\\n"\n' "$$line_esc" >> "$$out"
      done < "$$in"

      # End the C string
      printf ';\n' >> "$$out"

      echo "Output config:  $$out" 1>&2
    """,
    cmd_ps = r"""
      $in  = "$(location mlperf.conf)"
      $out = "$@"

      "const char* mlperf_conf =" | Out-File -FilePath $out -Encoding utf8

      Get-Content -LiteralPath $in | ForEach-Object {
        # Escape backslashes and quotes
        $line = $_.Replace('\', '\\').Replace('"', '\"')
        '"{0}\n"' -f $line | Out-File -FilePath $out -Encoding utf8 -Append
      }

      ";" | Out-File -FilePath $out -Encoding utf8 -Append
    """,
)

cc_library(
    name = "loadgen",
    srcs = [
        "loadgen/early_stopping.cc",
        "loadgen/issue_query_controller.cc",
        "loadgen/loadgen.cc",
        "loadgen/logging.cc",
        "loadgen/logging.h",
        "loadgen/results.cc",
        "loadgen/test_settings_internal.cc",
        "loadgen/test_settings_internal.h",
        "loadgen/utils.cc",
        "loadgen/utils.h",
        "loadgen/version.cc",
        "loadgen/version.h",
        "loadgen/version_generated.cc",
    ],
    hdrs = [
        ":mlperf_conf_h",
        "loadgen/early_stopping.h",
        "loadgen/issue_query_controller.h",
        "loadgen/loadgen.h",
        "loadgen/query_sample.h",
        "loadgen/query_sample_library.h",
        "loadgen/results.h",
        "loadgen/system_under_test.h",
        "loadgen/test_settings.h",
    ],
)
