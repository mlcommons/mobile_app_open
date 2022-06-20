import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:intl/intl.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class HistoryHelperUtils {
  final AppLocalizations l10n;

  HistoryHelperUtils(this.l10n);

  String formatDate(DateTime value) {
    var dateFormat = DateFormat('yyyy-MM-dd HH:mm');
    return dateFormat.format(value);
  }

  String formatDuration(int milliseconds) {
    var seconds = (milliseconds / Duration.millisecondsPerSecond).ceil();
    var minutes = seconds ~/ Duration.secondsPerMinute;
    seconds -= minutes * Duration.secondsPerMinute;
    final hours = minutes ~/ Duration.minutesPerHour;
    minutes -= hours * Duration.minutesPerHour;

    final tokens = <String>[];
    if (hours != 0) {
      tokens.add(hours.toString().padLeft(2, '0'));
    }
    tokens.add(minutes.toString().padLeft(2, '0'));
    tokens.add(seconds.toString().padLeft(2, '0'));

    return tokens.join(':');
  }

  AppBar makeAppBar(String title) {
    return AppBar(
      title: Text(
        title,
        style: TextStyle(fontSize: 24, color: AppColors.lightText),
      ),
      centerTitle: true,
      backgroundColor: AppColors.darkAppBarBackground,
      iconTheme: IconThemeData(color: AppColors.lightAppBarIconTheme),
    );
  }

  Widget makeHeader(String value) {
    return Center(
      child: Text(
        value,
        style: TextStyle(
          fontSize: 30,
          fontWeight: FontWeight.bold,
        ),
      ),
    );
  }

  Widget makeSubHeader(String value) {
    return Center(
      child: Padding(
        padding: EdgeInsets.symmetric(vertical: 15.0),
        child: Text(
          value,
          style: TextStyle(
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget makeInfo(String name, String value) {
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

  Widget makeTable(List<RowData> rows) {
    final borderStyle = BorderSide(
      width: 1,
      color: Colors.grey,
    );
    final headerBorder = TableBorder.all(
      width: 1,
      color: Colors.grey,
    );
    final rowBorder = TableBorder(
      left: borderStyle,
      right: borderStyle,
      bottom: borderStyle,
      verticalInside: borderStyle,
    );
    final headerStyle = TextStyle(fontWeight: FontWeight.bold);
    final table = Column(
      children: rows.map<Widget>((rowData) {
        final style = rowData.isHeader ? headerStyle : null;
        final perfStyle = rowData.throughputValid
            ? style
            : TextStyle(color: AppColors.darkRedText);
        final table = Table(
          border: rowData.onTap == null ? headerBorder : rowBorder,
          defaultVerticalAlignment: TableCellVerticalAlignment.middle,
          columnWidths: const {
            0: FlexColumnWidth(8),
            1: FlexColumnWidth(6),
            2: FlexColumnWidth(4.5),
          },
          children: [
            _makeTableRow([
              Text(rowData.name, style: style),
              Text(rowData.throughput, style: perfStyle),
              Text(rowData.accuracy, style: style),
            ])
          ],
        );
        return InkWell(
          onTap: rowData.onTap,
          child: table,
        );
      }).toList(),
    );
    return Padding(
      padding: EdgeInsets.symmetric(horizontal: 10),
      child: table,
    );
  }
}

class RowData {
  final bool isHeader;
  final String name;
  final String throughput;
  final bool throughputValid;
  final String accuracy;
  final void Function()? onTap;

  RowData({
    required this.isHeader,
    required this.name,
    required this.throughput,
    required this.throughputValid,
    required this.accuracy,
    required this.onTap,
  });
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
