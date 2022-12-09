import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:intl/intl.dart';
import 'package:mlperfbench_common/data/environment/env_android.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class HistoryHelperUtils {
  final AppLocalizations l10n;

  HistoryHelperUtils(this.l10n);

  String formatDate(DateTime value) {
    var dateFormat = DateFormat('yyyy-MM-dd HH:mm:ss');
    return dateFormat.format(value);
  }

  AppBar makeAppBar(
    String title, {
    Widget? leading,
    List<Widget>? actions,
    PreferredSizeWidget? bottom,
  }) {
    return AppBar(
      title: Text(
        title,
        style: const TextStyle(fontSize: 24, color: AppColors.lightText),
      ),
      centerTitle: true,
      backgroundColor: AppColors.darkAppBarBackground,
      iconTheme: const IconThemeData(color: AppColors.lightAppBarIconTheme),
      leading: leading,
      actions: actions,
      bottom: bottom,
    );
  }

  Widget makeHeader(String value) {
    return Center(
      child: Text(
        value,
        style: const TextStyle(
          fontSize: 30,
          fontWeight: FontWeight.bold,
        ),
      ),
    );
  }

  Widget makeSubHeader(String value) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.symmetric(vertical: 15.0),
        child: Text(
          value,
          style: const TextStyle(
            fontSize: 20,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  Widget makeInfo(String name, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 0.0),
      child: ListTile(
        minVerticalPadding: 0,
        title: Text(name),
        subtitle: GestureDetector(
          child: Text(
            value,
            style: const TextStyle(
              color: AppColors.darkText,
              fontWeight: FontWeight.bold,
              fontSize: 18.0,
            ),
          ),
          onTap: () async {
            await Clipboard.setData(ClipboardData(text: value));
            BotToast.showText(
              text: l10n.historyValueCopiedToast.replaceFirst('<name>', name),
              animationDuration: const Duration(milliseconds: 60),
              animationReverseDuration: const Duration(milliseconds: 60),
              duration: const Duration(seconds: 1),
            );
          },
        ),
      ),
    );
  }

  Widget makeListItem({
    required String title,
    String subtitle = '',
    Widget? trailing,
    void Function()? onTap,
    void Function()? onLongPress,
    bool specialTitleColor = false,
  }) {
    return ListTile(
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(
          title,
          style: TextStyle(
            fontWeight: FontWeight.bold,
            color: specialTitleColor ? AppColors.darkRedText : null,
          ),
        ),
      ),
      subtitle: subtitle.isEmpty
          ? null
          : Text(
              subtitle,
              style: const TextStyle(fontWeight: FontWeight.bold),
            ),
      trailing: trailing ?? const Icon(Icons.chevron_right),
      onTap: onTap,
      onLongPress: onLongPress,
    );
  }

  Widget makeTable(List<RowData> rows) {
    const borderStyle = BorderSide(
      width: 1,
      color: Colors.grey,
    );
    final headerBorder = TableBorder.all(
      width: 1,
      color: Colors.grey,
    );
    const rowBorder = TableBorder(
      left: borderStyle,
      right: borderStyle,
      bottom: borderStyle,
      verticalInside: borderStyle,
    );
    const headerStyle = TextStyle(fontWeight: FontWeight.bold);
    const rowStyle = TextStyle();
    final table = Column(
      children: rows.map<Widget>((rowData) {
        final style = rowData.isHeader ? headerStyle : rowStyle;
        final perfStyle = rowData.throughputValid
            ? style
            : const TextStyle(color: AppColors.darkRedText);
        final firstColumnText = Text(rowData.name, style: style);
        final firstColumn = rowData.isHeader
            ? firstColumnText
            : Row(
                children: [
                  Expanded(child: firstColumnText),
                  const Icon(Icons.chevron_right, color: Colors.grey),
                ],
              );
        final table = Table(
          border: rowData.onTap == null ? headerBorder : rowBorder,
          defaultVerticalAlignment: TableCellVerticalAlignment.middle,
          columnWidths: const {
            1: FixedColumnWidth(100),
            2: FixedColumnWidth(80),
          },
          children: [
            _makeTableRow([
              firstColumn,
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
      padding: const EdgeInsets.symmetric(horizontal: 10),
      child: table,
    );
  }

  String makeModelDescription(EnvironmentInfo info) {
    switch (info.platform) {
      case EnvPlatform.android:
        final android = info.value.android;
        if (android == null) {
          return 'Unknown Android device';
        }
        return '${android.manufacturer} ${android.modelName}';
      case EnvPlatform.ios:
        final ios = info.value.ios;
        if (ios == null) {
          return 'Unknown iOS device';
        }
        return 'Apple ${ios.modelName}';
      case EnvPlatform.windows:
        return 'PC';
      default:
        return '';
    }
  }

  String makeSocName(BenchmarkState state, EnvironmentInfo info) {
    switch (info.platform) {
      case EnvPlatform.android:
        final android = info.value.android;
        if (android == null) {
          return 'Unknown';
        }

        if (android.boardCode != null) {
          final boardName = state.boardDecoder.boards[android.boardCode];
          if (boardName != null) {
            return boardName;
          }
        }
        final socProp =
            android.props.where((e) => e.type == AndroidPropType.socName);
        if (socProp.isNotEmpty) {
          return socProp.first.value;
        }

        return android.procCpuinfoSocName;
      case EnvPlatform.ios:
        final ios = info.value.ios;
        if (ios == null) {
          return 'Unknown';
        }
        return ios.socName;
      case EnvPlatform.windows:
        final windows = info.value.windows;
        if (windows == null) {
          return 'Unknown';
        }
        return windows.cpuFullName;
      default:
        return 'Unknown';
    }
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
          padding: const EdgeInsets.all(8.0),
          child: cell,
        );
      },
    ).toList(),
  );
}
