import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/run_details_screen.dart';
import 'utils.dart';

class DetailsScreen extends StatefulWidget {
  final ExtendedResult result;

  const DetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  _DetailsScreen createState() => _DetailsScreen();
}

class _DetailsScreen extends State<DetailsScreen> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    return Scaffold(
      appBar: helper.makeAppBar(l10n.historyDetailsTitle),
      body: ListView(children: _makeBody()),
    );
  }

  List<Widget> _makeBody() {
    final res = widget.result;

    final firstResult = res.results.list.first;
    final date = helper.formatDate(firstResult.performance!.startDatetime);
    final backendName = firstResult.backendInfo.name;

    final averageThroughput =
        res.results.calculateAverageThroughput().toStringAsFixed(2);

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

    final modelDescription = res.envInfo.manufacturer.isEmpty
        ? res.envInfo.modelName
        : '${res.envInfo.manufacturer} ${res.envInfo.modelName}';
    final socDescription = res.envInfo.socInfo.cpuinfo.socName;

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
      makeBenchmarkTable(context, res.results.list),
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
      throughput: runInfo.performance?.throughput?.toStringAsFixed(2) ??
          l10n.resultsNotAvailable,
      throughputValid: runInfo.performance?.loadgenInfo?.validity ?? false,
      accuracy:
          runInfo.accuracy?.accuracy?.formatted ?? l10n.resultsNotAvailable,
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
