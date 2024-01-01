import 'package:mlperfbench/data/results/benchmark_result.dart';

enum SortByEnum { dateAsc, dateDesc, taskThroughputAsc, taskThroughputDesc }

class ResultSort {
  SortByEnum sortBy = SortByEnum.dateDesc;

  ResultSort();

  List<BenchmarkExportResult> apply(List<BenchmarkExportResult> items) {
    List<BenchmarkExportResult> sortedItems =
        List<BenchmarkExportResult>.from(items);
    switch (sortBy) {
      case SortByEnum.dateAsc:
        sortedItems.sort((a, b) => _sortByDates(a, b, true));
        break;
      case SortByEnum.dateDesc:
        sortedItems.sort((a, b) => _sortByDates(a, b, false));
        break;
      case SortByEnum.taskThroughputAsc:
        sortedItems.sort((a, b) => _sortByTaskThroughput(b, a));
        break;
      case SortByEnum.taskThroughputDesc:
        sortedItems.sort((a, b) => _sortByTaskThroughput(a, b));
        break;
    }
    return sortedItems;
  }

  int _sortByDates(
      BenchmarkExportResult a, BenchmarkExportResult b, bool isAsc) {
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

  int _sortByTaskThroughput(BenchmarkExportResult a, BenchmarkExportResult b) {
    const noThroughput = Throughput(value: 0);
    final aThroughput = a.performanceRun?.throughput ?? noThroughput;
    final bThroughput = b.performanceRun?.throughput ?? noThroughput;
    return bThroughput.compareTo(aThroughput);
  }
}
