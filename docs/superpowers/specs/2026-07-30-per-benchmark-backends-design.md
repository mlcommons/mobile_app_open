# Per-Benchmark Backend Selection — Design

Date: 2026-07-30
Status: approved (user delegated approval: "use recommended solution")

## Problem

The app picks exactly one backend library for the whole session
(`BackendInfoHelper.findMatching()`, `flutter/lib/backend/list.dart`). On iOS with
`WITH_APPLE=1`, the CoreML backend always matches and wins, so every benchmark runs
on `libcoremlbackend`, and benchmarks CoreML doesn't declare (all `llm-*`,
`stable_diffusion`) silently disappear from the app. There is no way to run
image classification on CoreML while LLM benchmarks run on TFLite.

## Goal

Each benchmark can run on a different backend library. On iOS: image
classification on `libcoremlbackend`, LLM benchmarks on `libtflitebackend`,
switchable per benchmark from the UI.

Decided with the user:

1. UX = per-benchmark backend dropdown in the benchmark config screen (same
   pattern as the existing delegate dropdown), with a smart default: each
   benchmark defaults to the highest-priority matching backend that supports it
   (priority = order in `flutter/lib/backend/list.in`, vendor/CoreML before
   TFLite). Benchmarks unsupported by the winning backend fall back to the next
   matching backend instead of being dropped.
2. Scope = all platforms, in shared Dart code. Where only one backend matches,
   behavior is unchanged. The fatal `'multiple matching backends found'` error
   is removed — multiple matches become the feature.
3. Switching a benchmark's backend switches its delegate choices, model files,
   batch size to the new backend's `benchmark_setting`.
4. Result export records the backend that actually ran, per benchmark.
5. Sequential only: one backend lib loaded per run (`ExternalBackend` already
   dlopens/dlcloses per run). No concurrent multi-backend runs.

## Architecture

No C++, proto, or result-JSON-schema changes. `dart_ffi_run_benchmark` already
builds a fresh `ExternalBackend` from `backend_lib_path` per run, and
`BenchmarkExportResult.backendInfo` is already per-result. All changes are Dart.

### Data model

`flutter/lib/backend/list.dart`:

- `findMatching()` → `findMatchingBackends()` returning `List<BackendInfo>` in
  priority order (= `_backendsList` order).
- Fallback policy is a mechanism/policy split: `const _alwaysOfferFallback`
  controls whether `libtflitebackend` is probed and appended even when a vendor
  backend matched. **Shipped `true`** — this is what makes the feature reachable
  (iOS: CoreML + TFLite both offered; vendor Android builds: TFLite offered as
  baseline and previously-hidden tasks appear defaulting to TFLite). Setting it
  `false` restores exactly today's semantics (fallback probed only when nothing
  matched).
- Keeps: `throw 'no matching backend found'`; `UnsupportedDeviceException`
  propagation (a vendor veto still blocks the app — unchanged).
- Removes: the multiple-match throw.
- `BackendInfo` gains a `@visibleForTesting` constructor.

`flutter/lib/benchmark/benchmark.dart`:

```dart
class BenchmarkBackend {
  final BackendInfo info;              // libName + BackendSetting (commonSetting)
  final pb.BenchmarkSetting settings;  // this backend's entry for this task
}

class Benchmark {
  final List<BenchmarkBackend> backends;   // priority order, length >= 1
  BenchmarkBackend _selected;              // defaults to backends.first
  pb.BenchmarkSetting get benchmarkSettings => _selected.settings; // was final field
  String get backendRequestDescription => benchmarkSettings.framework;
  BenchmarkBackend get selectedBackend => _selected;
  bool selectBackend(String libName);      // false if not a candidate
}
```

The getter conversion is the load-bearing move: `selectedDelegate`, the delegate
dropdown, `listResources`, run and export code all follow the selection with
zero edits. Each backend's proto carries its own `delegateSelected`, so
per-backend delegate choices are remembered in-session for free.

`createRunSettings` derives `commonSettings` and `backendLibName` from
`selectedBackend` internally; both parameters are removed from it and from
`_NativeRunHelper.initRunSettings`. A mismatched (settings, lib) pair becomes
unrepresentable.

`BenchmarkStore`: constructed from `List<BackendInfo> backends` +
`Map<String, String> backendSelection`. Per task, collect candidates from every
backend's `benchmark_setting` in priority order; skip the task only when no
backend supports it; apply persisted selection when valid (stale/unknown
libName → default + log, never fatal). Add
`Map<String, String> get backendSelectionMap`.

### Persistence

`Store.backendSelection`: JSON map `{benchmarkId: libName}` mirroring
`taskSelection`. Persisted because a forgotten selection silently changes which
backend runs and which large models get downloaded. Stale entries are ignored.

### Wiring

- `state.dart`: `late final List<BackendInfo> matchedBackends` replaces
  `backendInfo`; `setTaskConfig` parses `_store.backendSelection` and passes it
  to `BenchmarkStore`; new mutator `benchmarkSetBackend(benchmark, libName)`
  mirroring `benchmarkSetDelegate` (no-op while running/aborting; persists map;
  `notifyListeners`).
- `task_runner.dart`: `backendInfo` field removed; run settings come from the
  benchmark itself; `ResultHelper` built without `backendInfo`.
- `export_result_helper.dart`: `filename: runInfo.settings.backend_lib_name` —
  the export records what actually ran (snapshot taken at run start), not any
  live object.
- UI `benchmark_config_section.dart`: `_backendDescription` (static framework
  text) becomes `_backendChoice`: one candidate → today's exact static text;
  more → a `DropdownButton` cloned from `_delegateChoice` styling. Labels come
  from each backend's `settings.framework` pbtxt value, disambiguated with a
  cleaned libName when labels collide; the selection key is always `libName`.

### Correctness fixes folded in (verified pre-existing or multi-backend bugs)

1. **Symlink dir namespacing**: `benchmark.dart` names the model symlink dir by
   delegate name only; two backends sharing a delegate name would
   cross-contaminate models. New dir name:
   `'${libName}_${benchmarkId}_$delegateName'` (spaces → `_`). Orphaned dirs are
   cleaned by the existing app-version cache purge.
2. **Idempotent customSetting merge**: `createRunSettings` unconditionally
   `addAll`s task custom configs into the proto on every call (duplicate
   append). Merge now skips ids already present.
3. **Mixed-backend consumers**: `result_filter.dart` matches any result's
   backend (was: first result only); `extended_result_screen.dart` shows
   comma-joined distinct backend names; `integration_test/utils.dart`
   `canRunBenchmark` gates on that benchmark's selected backend.

### Explicitly out of scope

- Concurrent multi-backend runs (backend C++ globals forbid it; sequential
  dlopen/dlclose per run is the existing mechanism).
- Downgrading `UnsupportedDeviceException` to per-backend exclusion.
- Official-submission policy for mixed-backend result files (flagged for
  MLCommons review; schema is unchanged and already per-result).

## Testing

- Unit (`flutter/test/unit_test/`): backend list matching (fallback policies,
  ordering, no-match throw), benchmark store (candidates, defaults, persisted +
  stale selection, `selectBackend` swapping settings/delegates, `listResources`
  follows selection, idempotent merge), export filename from RunSettings
  snapshot, store round-trip, mixed-backend result filter.
- Widget: static text for one candidate; dropdown + callback for two.
- Existing suites must stay green; single-backend behavior byte-identical apart
  from the fallback now being offered.
