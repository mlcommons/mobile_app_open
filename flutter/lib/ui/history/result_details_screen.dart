import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:tuple/tuple.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'run_details_screen.dart';

class DetailsScreen extends StatefulWidget {
  final ExtendedResult result;

  const DetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  _DetailsScreen createState() => _DetailsScreen();
}

class _DetailsScreen extends State<DetailsScreen> {
  late AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);

    var dateFormat = DateFormat('yyyy-MM-dd HH:mm');

    final appVersionType = (widget.result.buildInfo.gitDirtyFlag ||
            widget.result.buildInfo.devTestFlag)
        ? l10n.historyDetailsBuildTypeDebug
        : widget.result.buildInfo.officialReleaseFlag
            ? l10n.historyDetailsBuildTypeOfficial
            : l10n.historyDetailsBuildTypeUnofficial;
    final date = dateFormat
        .format(widget.result.results.list.first.performance!.startDatetime);
    final averageThroughput =
        widget.result.results.calculateAverageThroughput().toStringAsFixed(2);
    final appVersion = l10n.historyDetailsAppVersionTemplate
        .replaceFirst('<version>', widget.result.buildInfo.version)
        .replaceFirst('<build>', widget.result.buildInfo.buildNumber)
        .replaceFirst('<buildType>', appVersionType);
    final backendName = widget.result.results.list.first.backendInfo.name;

    return Scaffold(
      appBar: AppBar(
        title: Text(
          l10n.historyDetailsTitle,
          style: TextStyle(fontSize: 24, color: AppColors.lightText),
        ),
        centerTitle: true,
        backgroundColor: AppColors.darkAppBarBackground,
        iconTheme: IconThemeData(color: AppColors.lightAppBarIconTheme),
      ),
      body: ListView(padding: const EdgeInsets.only(top: 0), children: [
        _makeInfo(l10n.historyDetailsDate, date),
        _makeInfo(l10n.historyDetailsUUID, widget.result.meta.uuid),
        _makeInfo(l10n.historyDetailsAvgQps, averageThroughput),
        _makeInfo(l10n.historyDetailsAppVersion, appVersion),
        _makeInfo(l10n.historyDetailsBackendName, backendName),
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
              fontSize: 18.0,
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

  Widget _makeResultTable() {
    return _makeTable(
      [
            Tuple2([
              l10n.historyDetailsTableColName,
              l10n.historyDetailsTableColPerf,
              l10n.historyDetailsTableColAccuracy,
              // ignore: unnecessary_cast
            ], null as void Function()?)
          ] +
          widget.result.results.list.map((runInfo) {
            return Tuple2([
              runInfo.benchmarkName,
              runInfo.performance?.throughput?.toStringAsFixed(2) ?? l10n.notAvailable,
              runInfo.accuracy?.accuracy?.formatted ?? l10n.notAvailable,
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
          border: onTap == null
              ? TableBorder.all(
                  width: 1,
                  color: Colors.blue,
                )
              : TableBorder(
                  left: BorderSide(
                    width: 1,
                    color: Colors.blue,
                  ),
                  right: BorderSide(
                    width: 1,
                    color: Colors.blue,
                  ),
                  bottom: BorderSide(
                    width: 1,
                    color: Colors.blue,
                  ),
                  verticalInside: BorderSide(
                    width: 1,
                    color: Colors.blue,
                  ),
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
