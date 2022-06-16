import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:tuple/tuple.dart';

import 'package:mlperfbench/app_constants.dart';
import 'run_details_screen.dart';

// import 'package:mlperfbench/localizations/app_localizations.dart';

class DetailsScreen extends StatefulWidget {
  final ExtendedResult result;

  const DetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  _DetailsScreen createState() => _DetailsScreen();
}

class _DetailsScreen extends State<DetailsScreen> {
  @override
  Widget build(BuildContext context) {
    // final stringResources = AppLocalizations.of(context);

    var dateFormat = DateFormat('yyyy-MM-dd HH:mm');

    final releaseVersion = widget.result.buildInfo.officialReleaseFlag &&
        !widget.result.buildInfo.gitDirtyFlag &&
        !widget.result.buildInfo.devTestFlag;
    final date = dateFormat
        .format(widget.result.results.list.first.performance!.startDatetime);
    final averageThroughput =
        widget.result.results.calculateAverageThroughput().toStringAsFixed(2);
    final appVersion =
        '${widget.result.buildInfo.version} (build ${widget.result.buildInfo.buildNumber})' +
            (releaseVersion ? '' : ' (unofficial)');
    final backendName = widget.result.results.list.first.backendInfo.name;

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
        _makeInfo('Date', date),
        _makeInfo('UUID', widget.result.meta.uuid),
        _makeInfo('Average throughput', averageThroughput),
        _makeInfo('App version', appVersion),
        _makeInfo('Backend', backendName),
        _makeResultTable(),
      ]),
    );
  }

  Widget _makeInfo(String name, String value) {
    return Padding(
      padding: EdgeInsets.symmetric(vertical: 0.0),
      child: ListTile(
        // visualDensity: VisualDensity(vertical: VisualDensity.minimumDensity),
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

  Widget _makeResultTable() {
    return _makeTable(
      [
            Tuple2([
              'Benchmark name',
              'Throughput',
              'Accuracy',
              // ignore: unnecessary_cast
            ], null as void Function()?)
          ] +
          widget.result.results.list.map((runInfo) {
            return Tuple2([
              runInfo.benchmarkName,
              runInfo.performance?.throughput?.toStringAsFixed(2) ?? 'N/A',
              runInfo.accuracy?.accuracy?.formatted ?? 'N/A',
            ], () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => RunDetailsScreen(result: runInfo),
                ),
              );
            });
          }).toList(),
    );
  }

  Widget _makeTable(List<Tuple2<List<String>, void Function()?>> rows) {
    return (Column(
      children: rows.map<Widget>((rowData) {
        final rowValues = rowData.item1;
        final onTap = rowData.item2;
        final table = Table(
          border: TableBorder.all(
            width: 1,
            color: Colors.blue,
          ),
          defaultVerticalAlignment: TableCellVerticalAlignment.middle,
          columnWidths: const {
            0: FlexColumnWidth(10),
            1: FlexColumnWidth(5),
            2: FlexColumnWidth(5),
          },
          children: [
            _makeTableRow(rowValues.map((value) {
              return Text(
                value,
                style: onTap == null
                    ? TextStyle(fontWeight: FontWeight.bold)
                    : null,
              );
            }).toList())
          ],
        );
        return InkWell(
          onTap: onTap,
          child: table,
        );
      }).toList(),
    ));
  }

  TableRow _makeTableRow(List<Widget> cells) {
    return TableRow(
      children: cells.map(
        (cell) {
          return Padding(
            padding: EdgeInsets.all(8.0),
            child: cell,
          );
        },
      ).toList(),
    );
  }
}
