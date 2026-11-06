import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

class _FakeBackendInfoHelper extends BackendInfoHelper {
  final List<String> backends;
  final Set<String> matching;

  _FakeBackendInfoHelper({required this.backends, required this.matching});

  @override
  List<String> getBackendsList() => backends;

  @override
  pb.BackendSetting? match(String libName) {
    if (matching.contains(libName)) {
      return pb.BackendSetting(
        benchmarkSetting: [pb.BenchmarkSetting(benchmarkId: 'bm-$libName')],
      );
    }
    return null;
  }
}

void main() {
  group('BackendInfoHelper.findMatchingBackends', () {
    const vendor1 = 'libvendor1backend';
    const vendor2 = 'libvendor2backend';
    const tflite = 'libtflitebackend';
    const allBackends = [vendor1, vendor2, tflite];

    test('the fallback is always matched, after the vendors', () {
      // Per-benchmark visibility is decided later in BenchmarkStore by each
      // backend's claim_policy; the match list always carries the fallback.
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1, tflite]);
    });

    test('multiple vendor matches are all offered, in list order', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, vendor2, tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1, vendor2, tflite]);
    });

    test('fallback-only build returns just the fallback', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [tflite]);
    });

    test('vendors match even when the fallback is not built in', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1]);
    });

    test('no matching backend throws', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {},
      );
      expect(() => helper.findMatchingBackends(), throwsA(isA<String>()));
    });

    test('settings are carried through per backend', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(
        matches.first.settings.benchmarkSetting.first.benchmarkId,
        'bm-$vendor1',
      );
    });
  });
}
