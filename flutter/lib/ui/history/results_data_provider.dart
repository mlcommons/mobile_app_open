import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/result_filter.dart';
import 'package:mlperfbench_common/data/result_sort.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/ui/history/result_item.dart';

class ResultsDataProvider {
  List<ExtendedResult> results;
  ResultFilter filter;

  ResultsDataProvider(this.results, this.filter);

  List<ResultItem> resultItems() {
    results = results.where((result) => filter.match(result)).toList();
    if (filter.sortBy == SortByValues.taskThroughputDesc) {
      List<BenchmarkExportResult> benchmarks = results
          .expand((item) => item.results)
          .where((element) => element.performanceRun != null &&
                  element.performanceRun!.throughput != null &&
                  filter.benchmarkId != null
              ? element.benchmarkId == filter.benchmarkId
              : true)
          .toList();

      return ResultSort.sortBenchmarks(
              filter.sortBy ?? SortByValues.taskThroughputDesc, benchmarks)
          .map((item) => ResultItem<BenchmarkExportResult>(item))
          .toList();
    }

    return ResultSort.sortResults(
            filter.sortBy ?? SortByValues.dateDesc, results)
        .map((item) => ResultItem<ExtendedResult>(item))
        .toList();
  }
}
