import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/result_filter.dart';
import 'package:mlperfbench/data/result_sort.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';

class BenchmarksDataProvider {
  List<ExtendedResult> results;

  BenchmarksDataProvider(this.results);

  List<BenchmarkExportResult> resultItems(
      ResultFilter filter, ResultSort sort) {
    final benchmarks = results
        .where((result) => filter.match(result))
        .expand((item) => item.results)
        .where((benchmark) => filter.matchBenchmark(benchmark))
        .toList();
    return sort.apply(benchmarks);
  }
}
