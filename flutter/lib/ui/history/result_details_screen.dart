import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/run_details_screen.dart';
import 'utils.dart';

class DetailsScreen extends StatefulWidget {
  final ExtendedResult result;

  const DetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  State<DetailsScreen> createState() => _DetailsScreen();
}

class _DetailsScreen extends State<DetailsScreen> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;
  late BenchmarkState state;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);
    state = context.watch<BenchmarkState>();

    return Scaffold(
      appBar: helper.makeAppBar(l10n.historyDetailsTitle),
      body: ListView(children: _makeBody()),
    );
  }

  double calculateAverageThroughput(List<BenchmarkExportResult> results) {
    var throughput = 0.0;
    var count = 0;
    for (var item in results) {
      if (item.performanceRun == null) {
        continue;
      }
      throughput += item.performanceRun!.throughput!;
      count++;
    }
    return throughput / count;
  }

  List<Widget> _makeBody() {
    final res = widget.result;

    final firstResult = res.results.first;
    final date = helper.formatDate(res.meta.creationDate);
    final backendName = firstResult.backendInfo.backendName;

    final averageThroughput =
        calculateAverageThroughput(res.results).toStringAsFixed(2);

    final appVersionType =
        (res.buildInfo.gitDirtyFlag || res.buildInfo.devTestFlag)
            ? l10n.historyDetailsBuildTypeDebug
            : res.buildInfo.officialReleaseFlag
                ? l10n.historyDetailsBuildTypeOfficial
                : l10n.historyDetailsBuildTypeUnofficial;
    final appVersion = l10n.historyDetailsAppVersionTemplate
        .replaceFirst('<version>', res.buildInfo.version)
        .replaceFirst('<build>', res.buildInfo.buildNumber)
        .replaceFirst('<buildType>', appVersionType);

    final utils = HistoryHelperUtils(l10n);
    final modelDescription = utils.makeModelDescription(res.environmentInfo);
    final socDescription = utils.makeSocName(state, res.environmentInfo);

    return [
      helper.makeInfo(l10n.historyDetailsDate, date),
      helper.makeInfo(l10n.historyDetailsUUID, res.meta.uuid),
      helper.makeInfo(l10n.historyDetailsAvgQps, averageThroughput),
      helper.makeInfo(l10n.historyDetailsAppVersion, appVersion),
      helper.makeInfo(l10n.historyDetailsBackendName, backendName),
      helper.makeInfo(l10n.historyDetailsModelName, modelDescription),
      helper.makeInfo(l10n.historyDetailsSocName, socDescription),
      const Divider(),
      helper.makeHeader(l10n.historyDetailsTableTitle),
      makeBenchmarkTable(context, res.results),
    ];
  }

  Widget makeBenchmarkTable(
    BuildContext context,
    List<BenchmarkExportResult> list,
  ) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 15),
      child: helper.makeTable(
        [
          RowData(
              isHeader: true,
              name: l10n.historyDetailsTableColName,
              throughput: l10n.historyDetailsTableColPerf,
              throughputValid: true,
              accuracy: l10n.historyDetailsTableColAccuracy,
              onTap: null),
          ...list.map(_makeRowData).toList(),
        ],
      ),
    );
  }

  RowData _makeRowData(BenchmarkExportResult runInfo) {
    return RowData(
      isHeader: false,
      name: runInfo.benchmarkName,
      throughput: runInfo.performanceRun?.throughput?.toStringAsFixed(2) ??
          l10n.resultsNotAvailable,
      throughputValid: runInfo.performanceRun?.loadgenInfo?.validity ?? false,
      accuracy:
          runInfo.accuracyRun?.accuracy?.formatted ?? l10n.resultsNotAvailable,
      onTap: () {
        Navigator.push(
          context,
          MaterialPageRoute(
            builder: (context) => RunDetailsScreen(result: runInfo),
          ),
        );
      },
    );
  }
}
