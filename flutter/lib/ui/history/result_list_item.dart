import 'package:flutter/material.dart';

import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'list_item.dart';

class ExtendedResultListItem implements ListItem {
  final ExtendedResult item;
  final void Function()? tapHandler;

  ExtendedResultListItem(this.item, this.tapHandler);

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final helper = HistoryHelperUtils(l10n);

    return ListTile(
      title: Text(
        helper.formatDate(item.meta.creationDate.toLocal()),
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
        subtitle: Text('${itemScore()}\n${itemAdditionalInfo()}'),
        trailing: Text(itemDateTime()),
        isThreeLine: true,
        onTap: tapHandler,
      );

  double itemScore() {
    final prScore = item.performanceRun?.throughput?.value;
    final arScore = item.accuracyRun?.throughput?.value;
    return prScore ?? arScore ?? 0;
  }

  String itemDateTime() {
    final prDateTime = item.performanceRun?.startDatetime;
    final arDateTime = item.accuracyRun?.startDatetime;

    return DateFormat('yyyy-MM-dd HH:mm:ss')
        .format(prDateTime ?? arDateTime ?? DateTime.now());
  }

  String itemAdditionalInfo() {
    return '(Backend: ${item.backendInfo.backendName} | Accelerator: ${item.backendInfo.acceleratorName} | Delegate: ${item.backendSettings.delegate})';
  }
}
