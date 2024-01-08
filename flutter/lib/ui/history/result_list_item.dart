import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/ui/history/list_item.dart';
import 'package:mlperfbench/ui/icons.dart';
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
  Widget build(BuildContext context) {
    final leadingWidth = 0.08 * MediaQuery.of(context).size.width;
    final subtitleWidth = 0.64 * MediaQuery.of(context).size.width;
    final trailingWidth = 0.28 * MediaQuery.of(context).size.width;
    return ListTile(
      contentPadding: const EdgeInsets.fromLTRB(10, 8, 10, 16),
      minVerticalPadding: 0,
      leading: SizedBox(
        width: leadingWidth,
        height: leadingWidth,
        child: BenchmarkIcons.getDarkIcon(item.benchmarkId),
      ),
      title: SizedBox(
        width: subtitleWidth,
        child: Text(
          item.benchmarkName,
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
      ),
      subtitle: SizedBox(
        width: subtitleWidth,
        child: Text(
          '${itemAdditionalInfo()}\n${itemDateTime()}',
          style: const TextStyle(fontWeight: FontWeight.normal, height: 1.4),
        ),
      ),
      trailing: SizedBox(
          width: trailingWidth,
          child: Row(
            mainAxisAlignment: MainAxisAlignment.end,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Flexible(
                flex: 8,
                fit: FlexFit.tight,
                child: Text(
                  itemScore(),
                  style: const TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 16.0,
                    color: AppColors.resultValid,
                  ),
                ),
              ),
              const Flexible(
                flex: 2,
                fit: FlexFit.tight,
                child: Icon(Icons.chevron_right),
              ),
            ],
          )),
      onTap: tapHandler,
    );
  }

  String itemScore() {
    final throughput = item.performanceRun?.throughput;
    final accuracy = item.accuracyRun?.accuracy;
    var throughputString = 'n/a';
    var accuracyString = 'n/a';
    if (throughput != null) {
      throughputString = throughput.toUIString();
    }
    if (accuracy != null) {
      accuracyString = accuracy.formatted;
    }
    return '$throughputString\n$accuracyString';
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
