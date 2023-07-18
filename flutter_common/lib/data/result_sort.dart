import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/data/sort_by_item.dart';

class ResultSort {
  SortByValues? sortBy;
  ResultSort();

  List<BenchmarkExportResult> apply(List<BenchmarkExportResult> items) {
    List<BenchmarkExportResult> sortedItems =
        List<BenchmarkExportResult>.from(items);
    switch (sortBy) {
      case SortByValues.dateAsc:
        sortedItems.sort((a, b) => sortByDates(a, b, true));
        break;
      case SortByValues.dateDesc:
        sortedItems.sort((a, b) => sortByDates(a, b, false));
        break;
      case SortByValues.taskThroughputDesc:
        sortedItems.sort((a, b) => sortByTaskThroughput(a, b));
        break;
      default:
    }
    return sortedItems;
  }
}

int sortByDates(BenchmarkExportResult a, BenchmarkExportResult b, bool isAsc) {
  DateTime aDateTime = a.performanceRun?.startDatetime ??
      a.accuracyRun?.startDatetime ??
      DateTime.now();
  DateTime bDateTime = b.performanceRun?.startDatetime ??
      b.accuracyRun?.startDatetime ??
      DateTime.now();

  if (isAsc) {
    return aDateTime.compareTo(bDateTime);
  }
  return bDateTime.compareTo(aDateTime);
}

int sortByTaskThroughput(BenchmarkExportResult a, BenchmarkExportResult b) {
  final aThroughput = a.performanceRun?.throughput;
  final bThroughput = b.performanceRun?.throughput;

  if (aThroughput != null && bThroughput != null) {
    return bThroughput.compareTo(aThroughput);
  }
  if (aThroughput == null && bThroughput != null) {
    return 1;
  }
  if (aThroughput != null && bThroughput == null) {
    return -1;
  }
  return 0;
}
