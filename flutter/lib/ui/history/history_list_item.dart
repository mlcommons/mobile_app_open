import 'package:flutter/material.dart';

import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/formatter.dart';
import 'package:mlperfbench/ui/history/list_item.dart';

class HistoryListItem implements ListItem {
  final ExtendedResult item;
  final void Function()? tapHandler;

  HistoryListItem(this.item, this.tapHandler);

  @override
  Widget build(BuildContext context) {
    final leadingWidth = 0.08 * MediaQuery.of(context).size.width;
    final subtitleWidth = 0.64 * MediaQuery.of(context).size.width;
    final trailingWidth = 0.28 * MediaQuery.of(context).size.width;
    return ListTile(
      contentPadding: const EdgeInsets.fromLTRB(10, 8, 10, 16),
      minVerticalPadding: 0,
      leading: SizedBox(
        width: leadingWidth,
        height: leadingWidth,
        child: const Text('leading'),
      ),
      title: SizedBox(
        width: subtitleWidth,
        child: Text(
          item.meta.uuid,
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
      ),
      subtitle: SizedBox(
        width: subtitleWidth,
        child: Text(
          '${itemAdditionalInfo()}\n${itemDateTime()}',
          style: const TextStyle(fontWeight: FontWeight.normal, height: 1.4),
        ),
      ),
      trailing: SizedBox(
          width: trailingWidth,
          child: Row(
            mainAxisAlignment: MainAxisAlignment.end,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: const [
              Flexible(
                flex: 8,
                fit: FlexFit.tight,
                child: Text(
                  'itemScore()',
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                    fontSize: 16.0,
                    color: AppColors.resultValidText,
                  ),
                ),
              ),
              Flexible(
                flex: 2,
                fit: FlexFit.tight,
                child: Icon(Icons.chevron_right),
              ),
            ],
          )),
      onTap: tapHandler,
    );
  }

  String itemDateTime() {
    return item.meta.creationDate.toUIString();
  }

  String itemAdditionalInfo() {
    final backendName = item.results.first.backendInfo.backendName;
    final delegateName = item.results.first.backendSettings.delegate;
    final acceleratorName = item.results.first.backendInfo.acceleratorName;
    // This matched the UI in ResultScreen._createListOfBenchmarkResultBottomWidgets()
    final backendInfo = '$backendName | $delegateName | $acceleratorName';
    return backendInfo;
  }
}
