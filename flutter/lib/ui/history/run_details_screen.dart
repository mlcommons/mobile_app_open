import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:tuple/tuple.dart';

import 'package:mlperfbench/app_constants.dart';

// import 'package:mlperfbench/localizations/app_localizations.dart';

class RunDetailsScreen extends StatefulWidget {
  final BenchmarkExportResult result;

  const RunDetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  _RunDetailsScreen createState() => _RunDetailsScreen();
}

class _RunDetailsScreen extends State<RunDetailsScreen> {
  @override
  Widget build(BuildContext context) {
    // final stringResources = AppLocalizations.of(context);

    var dateFormat = DateFormat('yyyy-MM-dd HH:mm');

    return Scaffold(
      appBar: AppBar(
        title: Text(
          'Result details', // TODO move to resources
          style: TextStyle(fontSize: 24, color: AppColors.lightText),
        ),
        centerTitle: true,
        backgroundColor: AppColors.darkAppBarBackground,
        iconTheme: IconThemeData(color: AppColors.lightAppBarIconTheme),
      ),
      body: ListView(padding: const EdgeInsets.only(top: 0), children: [
        _makeInfo('Benchmark name', widget.result.benchmarkName),
        _makeInfo('Scenario', widget.result.loadgenScenario.toJson()),
        _makeInfo('Backend name', widget.result.backendInfo.name),
        _makeInfo('Vendor name', widget.result.backendInfo.vendor),
        _makeInfo('Accelerator', widget.result.backendInfo.accelerator),
        Divider(),
        Center(
          child: Text(
            'Performance run',
            style: TextStyle(
              fontSize: 30,
              fontWeight: FontWeight.bold,
            ),
          ),
        ),
        if (widget.result.performance != null) ...[
          _makeInfo(
              'Throughput',
              widget.result.performance!.throughput?.toStringAsFixed(2) ??
                  'N/A'),
          _makeInfo('Run is valid',
              widget.result.performance!.loadgenValidity.toString()),
          _makeInfo(
              'Duration',
              formatDuration(Duration(
                  milliseconds:
                      widget.result.performance!.measuredDurationMs.round()))),
          _makeInfo('Samples count',
              widget.result.performance!.measuredSamples.toString()),
          _makeInfo('Dataset type',
              widget.result.performance!.datasetInfo.type.toJson()),
          _makeInfo(
              'Dataset name', widget.result.performance!.datasetInfo.name),
        ] else ...[
          Center(
            child: Text(
              'N/A',
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
            'Accuracy run',
            style: TextStyle(
              fontSize: 30,
              fontWeight: FontWeight.bold,
            ),
          ),
        ),
        if (widget.result.accuracy != null) ...[
          _makeInfo(
              'Accuracy', widget.result.accuracy!.accuracy?.formatted ?? 'N/A'),
          _makeInfo(
              'Duration',
              formatDuration(Duration(
                  milliseconds:
                      widget.result.accuracy!.measuredDurationMs.round()))),
          _makeInfo('Samples count',
              widget.result.accuracy!.measuredSamples.toString()),
          _makeInfo('Dataset type',
              widget.result.accuracy!.datasetInfo.type.toJson()),
          _makeInfo('Dataset name', widget.result.accuracy!.datasetInfo.name),
        ] else ...[
          Center(
            child: Padding(
              padding: EdgeInsets.symmetric(vertical: 15.0),
              child: Text(
                'N/A',
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
    var seconds = d.inSeconds;
    final hours = seconds ~/ Duration.secondsPerHour;
    seconds -= hours * Duration.secondsPerHour;
    final minutes = seconds ~/ Duration.secondsPerMinute;
    seconds -= minutes * Duration.secondsPerMinute;

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
              color: Colors.black,
              fontWeight: FontWeight.bold,
            ),
          ),
          onTap: () async {
            await Clipboard.setData(ClipboardData(text: value));
            BotToast.showText(
              text: 'Value copied to clipboard',
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
