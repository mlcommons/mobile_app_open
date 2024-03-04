import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/backend/loadgen_info.dart';

// this data was extracted from one of real log files
class _Examples {
  static const mean =
      ':::MLLOG {"key": "result_mean_latency_ns", "value": 31329862, "time_ms": 273.005400, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/loadgen.cc", "line_no": 1119, "pid": 38760, "tid": 37352}}';
  static const meanKey = 'result_mean_latency_ns';
  static const max =
      ':::MLLOG {"key": "result_max_latency_ns", "value": 41013900, "time_ms": 273.005400, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/loadgen.cc", "line_no": 1118, "pid": 38760, "tid": 37352}}';
  static const queries =
      ':::MLLOG {"key": "result_query_count", "value": 8, "time_ms": 273.005400, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/loadgen.cc", "line_no": 1055, "pid": 38760, "tid": 37352}}';
  static const latency90 =
      ':::MLLOG {"key": "result_90.00_percentile_latency_ns", "value": 41013900, "time_ms": 273.005400, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/loadgen.cc", "line_no": 1124, "pid": 38760, "tid": 37352}}';
  static const validity =
      ':::MLLOG {"key": "result_validity", "value": "INVALID", "time_ms": 273.005400, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/loadgen.cc", "line_no": 1029, "pid": 38760, "tid": 37352}}';
  static const minDurationMet =
      ':::MLLOG {"key": "result_min_duration_met", "value": true, "time_ms": 41253.016458, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/results.cc", "line_no": 451, "pid": 33655, "tid": 13951061273224140806}}';
  static const minQueriesMet =
      ':::MLLOG {"key": "result_min_queries_met", "value": false, "time_ms": 41253.016458, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/results.cc", "line_no": 452, "pid": 33655, "tid": 13951061273224140806}}';
  static const earlyStoppingMet =
      ':::MLLOG {"key": "early_stopping_met", "value": true, "time_ms": 41253.016458, "namespace": "mlperf::logging", "event_type": "POINT_IN_TIME", "metadata": {"is_error": false, "is_warning": false, "file": "external/org_mlperf_inference/loadgen/results.cc", "line_no": 453, "pid": 33655, "tid": 13951061273224140806}}';
}

void main() {
  group('LoadgenInfo', () {
    test('extract keys', () async {
      const lines = [
        _Examples.mean,
        _Examples.max,
      ];

      final values = await LoadgenInfo.extractKeys(
        logLines: Stream.fromIterable(lines),
        requiredKeys: {_Examples.meanKey},
      );

      expect(values.length, 1);
      expect(values[_Examples.meanKey], 31329862);
    });
    test('extract keys: empty', () async {
      const lines = [
        _Examples.max,
      ];

      final values = await LoadgenInfo.extractKeys(
        logLines: Stream.fromIterable(lines),
        requiredKeys: {_Examples.meanKey},
      );

      expect(values.length, 0);
    });
    test('extract info', () async {
      const lines = [
        _Examples.mean,
        _Examples.latency90,
        _Examples.validity,
        _Examples.queries,
        _Examples.minDurationMet,
        _Examples.minQueriesMet,
        _Examples.earlyStoppingMet,
      ];

      final info = await LoadgenInfo.extractLoadgenInfo(
        logLines: Stream.fromIterable(lines),
      );

      expect(info, isNotNull);
      info!;
      expect(info.latency90, 0.0410139);
      expect(info.latencyMean, 0.031329862);
      expect(info.queryCount, 8);
      expect(info.isResultValid, false);
      expect(info.isMinDurationMet, true);
      expect(info.isMinQueryMet, false);
      expect(info.isEarlyStoppingMet, true);
    });
    test('extract info: missing keys: all', () async {
      const lines = [
        _Examples.max,
      ];

      final info = await LoadgenInfo.extractLoadgenInfo(
        logLines: Stream.fromIterable(lines),
      );

      expect(info, isNull);
    });
    test('extract info: missing keys: partially', () async {
      const lines = [
        _Examples.mean,
        _Examples.latency90,
        _Examples.validity,
      ];

      expect(() async {
        await LoadgenInfo.extractLoadgenInfo(
          logLines: Stream.fromIterable(lines),
        );
      }, throwsA(isA<TypeError>()));
    });
  });
}
