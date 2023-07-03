import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

enum SortByValues { dateAsc, dateDesc, taskThroughputDesc }

class ResultSort {
  ResultSort();

  static List<ExtendedResult> sortResults(
      SortByValues sortBy, List<ExtendedResult> items) {
    List<ExtendedResult> sortedItems = List<ExtendedResult>.from(items);
    switch (sortBy) {
      case SortByValues.dateAsc:
        sortedItems
            .sort((a, b) => a.meta.creationDate.compareTo(b.meta.creationDate));
        break;
      case SortByValues.dateDesc:
        sortedItems
            .sort((a, b) => b.meta.creationDate.compareTo(a.meta.creationDate));
        break;
      default:
    }
    return sortedItems;
  }

  static List<BenchmarkExportResult> sortBenchmarks(
      SortByValues sortBy, List<BenchmarkExportResult> items) {
    List<BenchmarkExportResult> sortedItems =
        List<BenchmarkExportResult>.from(items);
    switch (sortBy) {
      case SortByValues.taskThroughputDesc:
        items.sort((a, b) => b.performanceRun!.throughput!
            .compareTo(a.performanceRun!.throughput!));
        break;
      default:
    }
    return sortedItems;
  }
}
