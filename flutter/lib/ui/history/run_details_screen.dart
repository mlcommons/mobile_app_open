import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/data/results/loadgen_scenario.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'utils.dart';

class RunDetailsScreen extends StatefulWidget {
  final BenchmarkExportResult result;

  const RunDetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  _RunDetailsScreen createState() => _RunDetailsScreen();
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
      if (widget.result.performance != null)
        ..._makePerformanceInfo(widget.result.performance!)
      else
        helper.makeSubHeader(l10n.notAvailable),
      const Divider(),
      helper.makeHeader(l10n.historyRunDetailsAccuracyTitle),
      if (widget.result.accuracy != null)
        ..._makeAccuracyInfo(widget.result.accuracy!)
      else
        helper.makeSubHeader(l10n.notAvailable),
    ];
  }

  List<Widget> _makeMainInfo(BenchmarkExportResult res) {
    return [
      helper.makeInfo(l10n.historyRunDetailsBenchName, res.benchmarkName),
      helper.makeInfo(
          l10n.historyRunDetailsScenario, res.loadgenScenario.toJson()),
      helper.makeInfo(l10n.historyRunDetailsBackendName, res.backendInfo.name),
      helper.makeInfo(l10n.historyRunDetailsVendorName, res.backendInfo.vendor),
      helper.makeInfo(
          l10n.historyRunDetailsAccelerator, res.backendInfo.accelerator),
      if (res.loadgenScenario.value == LoadgenScenarioEnum.offline)
        helper.makeInfo(l10n.historyRunDetailsBatchSize,
            res.backendSettingsInfo.batchSize.toString()),
    ];
  }

  List<Widget> _makePerformanceInfo(BenchmarkRunResult perf) {
    return [
      helper.makeInfo(l10n.historyRunDetailsPerfQps,
          perf.throughput?.toStringAsFixed(2) ?? l10n.notAvailable),
      helper.makeInfo(
          l10n.historyRunDetailsValid, perf.loadgenValidity.toString()),
      helper.makeInfo(l10n.historyRunDetailsDuration,
          helper.formatDuration(perf.measuredDurationMs.ceil())),
      helper.makeInfo(
          l10n.historyRunDetailsSamples, perf.measuredSamples.toString()),
      helper.makeInfo(
          l10n.historyRunDetailsDatasetType, perf.datasetInfo.type.toJson()),
      helper.makeInfo(l10n.historyRunDetailsDatasetName, perf.datasetInfo.name),
    ];
  }

  List<Widget> _makeAccuracyInfo(BenchmarkRunResult accuracy) {
    return [
      helper.makeInfo(l10n.historyRunDetailsAccuracy,
          accuracy.accuracy?.formatted ?? l10n.notAvailable),
      helper.makeInfo(
        l10n.historyRunDetailsDuration,
        helper.formatDuration(accuracy.measuredDurationMs.ceil()),
      ),
      helper.makeInfo(
          l10n.historyRunDetailsSamples, accuracy.measuredSamples.toString()),
      helper.makeInfo(l10n.historyRunDetailsDatasetType,
          accuracy.datasetInfo.type.toJson()),
      helper.makeInfo(
          l10n.historyRunDetailsDatasetName, accuracy.datasetInfo.name),
    ];
  }
}
