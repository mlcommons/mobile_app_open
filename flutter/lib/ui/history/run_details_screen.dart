import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class RunDetailsScreen extends StatefulWidget {
  final BenchmarkExportResult result;

  const RunDetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  _RunDetailsScreen createState() => _RunDetailsScreen();
}

class _RunDetailsScreen extends State<RunDetailsScreen> {
  late AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);

    var dateFormat = DateFormat('yyyy-MM-dd HH:mm');

    return Scaffold(
      appBar: AppBar(
        title: Text(
          l10n.historyRunDetailsTitle,
          style: TextStyle(fontSize: 24, color: AppColors.lightText),
        ),
        centerTitle: true,
        backgroundColor: AppColors.darkAppBarBackground,
        iconTheme: IconThemeData(color: AppColors.lightAppBarIconTheme),
      ),
      body: ListView(padding: const EdgeInsets.only(top: 0), children: [
        _makeInfo(l10n.historyRunDetailsBenchName, widget.result.benchmarkName),
        _makeInfo(l10n.historyRunDetailsScenario,
            widget.result.loadgenScenario.toJson()),
        _makeInfo(
            l10n.historyRunDetailsBackendName, widget.result.backendInfo.name),
        _makeInfo(
            l10n.historyRunDetailsVendorName, widget.result.backendInfo.vendor),
        _makeInfo(l10n.historyRunDetailsAccelerator,
            widget.result.backendInfo.accelerator),
        if (widget.result.backendSettingsInfo.batchSize > 0)
          _makeInfo(l10n.historyRunDetailsBatchSize,
              widget.result.backendSettingsInfo.batchSize.toString()),
        Divider(),
        Center(
          child: Text(
            l10n.historyRunDetailsPerfTitle,
            style: TextStyle(
              fontSize: 30,
              fontWeight: FontWeight.bold,
            ),
          ),
        ),
        if (widget.result.performance != null) ...[
          _makeInfo(
              l10n.historyRunDetailsPerfQps,
              widget.result.performance!.throughput?.toStringAsFixed(2) ??
                  l10n.notAvailable),
          _makeInfo(l10n.historyRunDetailsValid,
              widget.result.performance!.loadgenValidity.toString()),
          _makeInfo(
              l10n.historyRunDetailsDuration,
              formatDuration(Duration(
                  milliseconds:
                      widget.result.performance!.measuredDurationMs.round()))),
          _makeInfo(l10n.historyRunDetailsSamples,
              widget.result.performance!.measuredSamples.toString()),
          _makeInfo(l10n.historyRunDetailsDatasetType,
              widget.result.performance!.datasetInfo.type.toJson()),
          _makeInfo(l10n.historyRunDetailsDatasetName,
              widget.result.performance!.datasetInfo.name),
        ] else ...[
          Center(
            child: Text(
              l10n.notAvailable,
              style: TextStyle(
                fontSize: 20,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
        ],
        Divider(),
        Center(
          child: Text(
            l10n.historyRunDetailsAccuracyTitle,
            style: TextStyle(
              fontSize: 30,
              fontWeight: FontWeight.bold,
            ),
          ),
        ),
        if (widget.result.accuracy != null) ...[
          _makeInfo(l10n.historyRunDetailsAccuracy,
              widget.result.accuracy!.accuracy?.formatted ?? l10n.notAvailable),
          _makeInfo(
              l10n.historyRunDetailsDuration,
              formatDuration(Duration(
                  milliseconds:
                      widget.result.accuracy!.measuredDurationMs.round()))),
          _makeInfo(l10n.historyRunDetailsSamples,
              widget.result.accuracy!.measuredSamples.toString()),
          _makeInfo(l10n.historyRunDetailsDatasetType,
              widget.result.accuracy!.datasetInfo.type.toJson()),
          _makeInfo(l10n.historyRunDetailsDatasetName,
              widget.result.accuracy!.datasetInfo.name),
        ] else ...[
          Center(
            child: Padding(
              padding: EdgeInsets.symmetric(vertical: 15.0),
              child: Text(
                l10n.notAvailable,
                style: TextStyle(
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                ),
              ),
            ),
          ),
        ],
      ]),
    );
  }

  static String formatDuration(Duration d) {
    var milliseconds = d.inMilliseconds;
    final hours = milliseconds ~/ Duration.millisecondsPerHour;
    milliseconds -= hours * Duration.millisecondsPerHour;
    final minutes = milliseconds ~/ Duration.millisecondsPerMinute;
    milliseconds -= minutes * Duration.millisecondsPerMinute;
    final seconds = (milliseconds / Duration.millisecondsPerSecond).ceil();

    final tokens = <String>[];
    if (hours != 0) {
      tokens.add(hours.toString().padLeft(2, '0'));
    }
    tokens.add(minutes.toString().padLeft(2, '0'));
    tokens.add(seconds.toString().padLeft(2, '0'));

    return tokens.join(':');
  }

  Widget _makeInfo(String name, String value) {
    return Padding(
      padding: EdgeInsets.symmetric(vertical: 0.0),
      child: ListTile(
        minVerticalPadding: 0,
        title: Text(name),
        subtitle: GestureDetector(
          child: Text(
            value,
            style: TextStyle(
              color: AppColors.darkText,
              fontWeight: FontWeight.bold,
            ),
          ),
          onTap: () async {
            await Clipboard.setData(ClipboardData(text: value));
            BotToast.showText(
              text: l10n.historyValueCopiedToast.replaceFirst('<name>', name),
              animationDuration: Duration(milliseconds: 60),
              animationReverseDuration: Duration(milliseconds: 60),
              duration: Duration(seconds: 1),
            );
          },
        ),
      ),
    );
  }
}
