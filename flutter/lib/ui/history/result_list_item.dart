import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/ui/history/list_item.dart';
import 'package:mlperfbench/ui/time_utils.dart';

class ExtendedResultListItem implements ListItem {
  final ExtendedResult item;
  final void Function()? tapHandler;

  ExtendedResultListItem(this.item, this.tapHandler);

  @override
  Widget build(BuildContext context) {
    return ListTile(
      title: Text(
        formatDateTime(item.meta.creationDate.toLocal()),
        style: const TextStyle(fontWeight: FontWeight.bold),
      ),
      subtitle: Text(item.meta.uuid),
      onTap: tapHandler,
    );
  }
}

class BenchmarkListItem implements ListItem {
  final BenchmarkExportResult item;
  final void Function()? tapHandler;

  BenchmarkListItem(this.item, this.tapHandler);

  @override
  Widget build(BuildContext context) => ListTile(
        title: Text(
          item.benchmarkName,
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
        subtitle: Text(
          '${itemAdditionalInfo()}\n${itemDateTime()}',
          style: const TextStyle(fontWeight: FontWeight.normal, height: 1.4),
        ),
        trailing: Text(
          itemScore(),
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
        isThreeLine: true,
        onTap: tapHandler,
      );

  String itemScore() {
    final throughput = item.performanceRun?.throughput;
    final accuracy = item.accuracyRun?.accuracy;
    if (throughput != null) {
      return throughput.toUIString();
    } else if (accuracy != null) {
      return accuracy.formatted;
    } else {
      return 'unknown';
    }
  }

  String itemDateTime() {
    final prDateTime = item.performanceRun?.startDatetime;
    final arDateTime = item.accuracyRun?.startDatetime;
    if (prDateTime != null) {
      return formatDateTime(prDateTime);
    } else if (arDateTime != null) {
      return formatDateTime(arDateTime);
    } else {
      return 'unknown';
    }
  }

  String itemAdditionalInfo() {
    final backendName = item.backendInfo.backendName;
    final delegateName = item.backendSettings.delegate;
    final acceleratorName = item.backendInfo.acceleratorName;
    // This matched the UI in ResultScreen._createListOfBenchmarkResultBottomWidgets()
    final backendInfo = '$backendName | $delegateName | $acceleratorName';
    return backendInfo;
  }
}
