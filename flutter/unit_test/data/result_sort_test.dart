import 'package:flutter_test/flutter_test.dart';
import 'package:mlperfbench/data/extended_result.dart';

import 'package:mlperfbench/data/generation_helpers/sample_generator.dart';
import 'package:mlperfbench/data/result_sort.dart';

void main() {
  group('ResultSort', () {
    final generator = SampleGenerator();
    final result1 = generator.extendedResult;
    final result2 = generator.extendedResult;
    final result3 = generator.extendedResult;
    List<ExtendedResult> results = [result1, result2, result3];

    final earliestResult = result1.meta.uuid;
    final latestResult = result3.meta.uuid;

    test('sort by Date ASC (from oldest to most recent)', () {
      final sort = ResultSort();
      sort.sortBy = SortByEnum.dateAsc;
      expect(sort.apply(results).first.meta.uuid, earliestResult);
      expect(sort.apply(results).last.meta.uuid, latestResult);
    });

    test('sort by Date DESC (from most recent to oldest)', () {
      final sort = ResultSort();
      sort.sortBy = SortByEnum.dateDesc;
      expect(sort.apply(results).first.meta.uuid, latestResult);
      expect(sort.apply(results).last.meta.uuid, earliestResult);
    });
  });
}
