import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/history/benchmark_export_result_screen.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'package:mlperfbench/ui/time_utils.dart';

class RemoteExtendedResultScreen extends StatefulWidget {
  final String fileName;

  const RemoteExtendedResultScreen({Key? key, required this.fileName})
      : super(key: key);

  @override
  State<StatefulWidget> createState() => _RemoteExtendedResultScreenState();
}

class _RemoteExtendedResultScreenState
    extends State<RemoteExtendedResultScreen> {
  late AppLocalizations l10n;

  Future<ExtendedResult> downloadResult() =>
      FirebaseManager.instance.downloadResult(widget.fileName);

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.historyDetailsTitle),
        actions: [_deleteButton()],
      ),
      body: FutureBuilder(
        future: downloadResult(),
        builder: (context, AsyncSnapshot<ExtendedResult> snapshot) {
          Widget child;
          final result = snapshot.data;
          if (snapshot.hasData && result != null) {
            child = ExtendedResultView(result: result);
          } else if (snapshot.hasError) {
            child = Text('${snapshot.error}');
          } else {
            child = const CircularProgressIndicator();
          }
          return Container(color: Colors.white, child: Center(child: child));
        },
      ),
    );
  }

  Widget _deleteButton() {
    return IconButton(
      icon: const Icon(Icons.delete),
      tooltip: l10n.historyListSelectionDelete,
      onPressed: () async {
        final dialogResult = await showConfirmDialog(
          context,
          l10n.historyDetailsDeleteConfirm,
          title: l10n.historyDetailsDelete,
        );
        switch (dialogResult) {
          case ConfirmDialogAction.ok:
            await FirebaseManager.instance.deleteResult(widget.fileName);
            if (context.mounted) {
              Navigator.pop(context);
            }
            break;
          case null:
          case ConfirmDialogAction.cancel:
            break;
        }
      },
    );
  }
}

class LocalExtendedResultScreen extends StatefulWidget {
  final ExtendedResult result;

  const LocalExtendedResultScreen({Key? key, required this.result})
      : super(key: key);

  @override
  State<LocalExtendedResultScreen> createState() =>
      _LocalExtendedResultScreenState();
}

class _LocalExtendedResultScreenState extends State<LocalExtendedResultScreen> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;
  late BenchmarkState state;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);
    state = context.watch<BenchmarkState>();

    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.historyDetailsTitle),
        actions: [_deleteButton()],
      ),
      body: ExtendedResultView(result: widget.result),
    );
  }

  Widget _deleteButton() {
    return IconButton(
      icon: const Icon(Icons.delete),
      tooltip: l10n.historyListSelectionDelete,
      onPressed: () async {
        final dialogResult = await showConfirmDialog(
          context,
          l10n.historyDetailsDeleteConfirm,
          title: l10n.historyDetailsDelete,
        );
        switch (dialogResult) {
          case ConfirmDialogAction.ok:
            setState(() {
              state.resourceManager.resultManager.deleteResult(widget.result);
              Navigator.pop(context);
            });
            break;
          case null:
          case ConfirmDialogAction.cancel:
            break;
        }
      },
    );
  }
}

class ExtendedResultView extends StatefulWidget {
  final ExtendedResult result;

  const ExtendedResultView({Key? key, required this.result}) : super(key: key);

  @override
  State<ExtendedResultView> createState() => _ExtendedResultViewState();
}

class _ExtendedResultViewState extends State<ExtendedResultView> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;
  late BenchmarkState state;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);
    state = context.watch<BenchmarkState>();
    return ListView(children: _makeBody());
  }

  double calculateAverageThroughput(List<BenchmarkExportResult> results) {
    var throughput = 0.0;
    var count = 0;
    for (var item in results) {
      if (item.performanceRun == null) {
        continue;
      }
      throughput += item.performanceRun!.throughput!.value;
      count++;
    }
    return throughput / count;
  }

  List<Widget> _makeBody() {
    final res = widget.result;

    final firstResult = res.results.first;
    final date = res.meta.creationDate.toUIString();
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
    final modelDescription = res.environmentInfo.modelDescription;
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
      throughput: runInfo.performanceRun?.throughput?.toUIString() ??
          l10n.resultsNotAvailable,
      throughputValid: runInfo.performanceRun?.loadgenInfo?.validity ?? false,
      accuracy:
          runInfo.accuracyRun?.accuracy?.formatted ?? l10n.resultsNotAvailable,
      onTap: () {
        Navigator.push(
          context,
          MaterialPageRoute(
            builder: (context) => BenchmarkExportResultScreen(result: runInfo),
          ),
        );
      },
    );
  }
}
