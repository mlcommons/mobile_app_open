import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;

class _FakeBackendInfoHelper extends BackendInfoHelper {
  final List<String> backends;
  final Set<String> matching;
  final Map<String, pb.FallbackPolicy> policies;

  _FakeBackendInfoHelper({
    required this.backends,
    required this.matching,
    this.policies = const {},
  });

  @override
  List<String> getBackendsList() => backends;

  @override
  pb.BackendSetting? match(String libName) {
    if (matching.contains(libName)) {
      return pb.BackendSetting(
        benchmarkSetting: [pb.BenchmarkSetting(benchmarkId: 'bm-$libName')],
        fallbackPolicy: policies[libName],
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

    test('vendor match without opt-in suppresses the fallback (default)', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1]);
    });

    test('FALLBACK_COEXIST vendor offers the fallback, in priority order', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
        policies: {vendor1: pb.FallbackPolicy.FALLBACK_COEXIST},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1, tflite]);
    });

    test('multiple vendor matches are all offered, in list order', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, vendor2, tflite},
        policies: {
          vendor1: pb.FallbackPolicy.FALLBACK_COEXIST,
          vendor2: pb.FallbackPolicy.FALLBACK_COEXIST,
        },
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

    test('alwaysOfferFallback=false suppresses even an opted-in vendor', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
        policies: {vendor1: pb.FallbackPolicy.FALLBACK_COEXIST},
      );
      final matches = helper.findMatchingBackends(alwaysOfferFallback: false);
      expect(matches.map((e) => e.libName), [vendor1]);
    });

    test('FALLBACK_DISABLED vendor suppresses the fallback device-wide', () {
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
        policies: {vendor1: pb.FallbackPolicy.FALLBACK_DISABLED},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1]);
    });

    test('FALLBACK_FILL_GAPS keeps the fallback in the match list', () {
      // Per-benchmark filtering for FILL_GAPS happens in BenchmarkStore;
      // the fallback must stay matched so it can fill unsupported tasks.
      final helper = _FakeBackendInfoHelper(
        backends: allBackends,
        matching: {vendor1, tflite},
        policies: {vendor1: pb.FallbackPolicy.FALLBACK_FILL_GAPS},
      );
      final matches = helper.findMatchingBackends();
      expect(matches.map((e) => e.libName), [vendor1, tflite]);
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
