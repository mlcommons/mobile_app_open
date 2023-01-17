import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/data/results/dataset_info.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/time_utils.dart';
import 'utils.dart';

class RunDetailsScreen extends StatefulWidget {
  final BenchmarkExportResult result;

  const RunDetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  State<RunDetailsScreen> createState() => _RunDetailsScreen();
}

class _RunDetailsScreen extends State<RunDetailsScreen> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    return Scaffold(
      appBar: helper.makeAppBar(widget.result.benchmarkName),
      body: ListView(children: _makeBody()),
    );
  }

  List<Widget> _makeBody() {
    return [
      ..._makeMainInfo(widget.result),
      const Divider(),
      helper.makeHeader(l10n.historyRunDetailsPerfTitle),
      if (widget.result.performanceRun != null)
        ..._makePerformanceInfo(widget.result.performanceRun!)
      else
        helper.makeSubHeader(l10n.resultsNotAvailable),
      const Divider(),
      helper.makeHeader(l10n.historyRunDetailsAccuracyTitle),
      if (widget.result.accuracyRun != null)
        ..._makeAccuracyInfo(widget.result.accuracyRun!)
      else
        helper.makeSubHeader(l10n.resultsNotAvailable),
    ];
  }

  List<Widget> _makeMainInfo(BenchmarkExportResult res) {
    return [
      helper.makeInfo(l10n.historyRunDetailsBenchName, res.benchmarkName),
      helper.makeInfo(
          l10n.historyRunDetailsScenario, res.loadgenScenario.humanName),
      helper.makeInfo(
          l10n.historyRunDetailsBackendName, res.backendInfo.backendName),
      helper.makeInfo(
          l10n.historyRunDetailsVendorName, res.backendInfo.vendorName),
      helper.makeInfo(
          l10n.historyRunDetailsAccelerator, res.backendInfo.acceleratorName),
      if (res.loadgenScenario == LoadgenScenarioEnum.offline)
        helper.makeInfo(l10n.historyRunDetailsBatchSize,
            res.backendSettings.batchSize.toString()),
    ];
  }

  List<Widget> _makePerformanceInfo(BenchmarkRunResult perf) {
    return [
      helper.makeInfo(l10n.historyRunDetailsPerfQps,
          perf.throughput?.toUIString() ?? l10n.resultsNotAvailable),
      helper.makeInfo(l10n.historyRunDetailsValid,
          (perf.loadgenInfo?.validity ?? false).toString()),
      helper.makeInfo(l10n.historyRunDetailsDuration,
          formatDuration(perf.measuredDuration)),
      helper.makeInfo(
          l10n.historyRunDetailsSamples, perf.measuredSamples.toString()),
      helper.makeInfo(
          l10n.historyRunDetailsDatasetType, perf.dataset.type.humanName),
      helper.makeInfo(l10n.historyRunDetailsDatasetName, perf.dataset.name),
    ];
  }

  List<Widget> _makeAccuracyInfo(BenchmarkRunResult accuracy) {
    return [
      helper.makeInfo(l10n.historyRunDetailsAccuracy,
          accuracy.accuracy?.formatted ?? l10n.resultsNotAvailable),
      helper.makeInfo(
        l10n.historyRunDetailsDuration,
        formatDuration(accuracy.measuredDuration),
      ),
      helper.makeInfo(
          l10n.historyRunDetailsSamples, accuracy.measuredSamples.toString()),
      helper.makeInfo(
          l10n.historyRunDetailsDatasetType, accuracy.dataset.type.humanName),
      helper.makeInfo(l10n.historyRunDetailsDatasetName, accuracy.dataset.name),
    ];
  }
}
