import 'package:flutter/material.dart';

import 'package:collection/collection.dart';
import 'package:intl/intl.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/ui/history/list_item.dart';
import 'package:mlperfbench/ui/icons.dart';

class HistoryListItem implements ListItem {
  final ExtendedResult item;
  final void Function()? tapHandler;

  HistoryListItem(this.item, this.tapHandler);

  final dateFormat = DateFormat('yyyy-MM-dd HH:mm');

  @override
  Widget build(BuildContext context) {
    final subtitleWidth = 0.40 * MediaQuery.of(context).size.width;
    final trailingWidth = 0.60 * MediaQuery.of(context).size.width;
    return ListTile(
      contentPadding: const EdgeInsets.fromLTRB(10, 8, 10, 16),
      minVerticalPadding: 0,
      title: SizedBox(
        width: subtitleWidth,
        child: Text(
          item.environmentInfo.modelDescription,
          maxLines: 2,
          overflow: TextOverflow.ellipsis,
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
      ),
      subtitle: SizedBox(
        width: subtitleWidth,
        height: 24,
        child: FittedBox(
          fit: BoxFit.fitWidth,
          child: Text(
            dateFormat.format(item.meta.creationDate),
            style: const TextStyle(fontWeight: FontWeight.normal),
          ),
        ),
      ),
      trailing: SizedBox(
          width: trailingWidth,
          child: Row(
            mainAxisAlignment: MainAxisAlignment.end,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Flexible(
                flex: 9,
                fit: FlexFit.tight,
                child: _resultList(),
              ),
              const Flexible(
                flex: 1,
                fit: FlexFit.tight,
                child: Icon(Icons.chevron_right),
              ),
            ],
          )),
      onTap: tapHandler,
    );
  }

  Widget _resultList() {
    List<Widget> children = [];
    for (String id in BenchmarkId.allIds) {
      final benchmark =
          item.results.firstWhereOrNull((e) => e.benchmarkId == id);
      final icon = SizedBox(
        width: 20,
        height: 20,
        child: BenchmarkIcons.getDarkIcon(id),
      );
      Widget iconWidget;
      if (benchmark != null) {
        iconWidget = icon;
      } else {
        iconWidget = ColorFiltered(
          colorFilter: _matrixColorFilter,
          child: icon,
        );
      }
      final throughput = benchmark?.performanceRun?.throughput;
      final accuracy = benchmark?.accuracyRun?.accuracy;
      var throughputString = 'n/a';
      var accuracyString = 'n/a';
      if (throughput != null) {
        throughputString = throughput.value.toStringAsFixed(0);
      }
      if (accuracy != null) {
        accuracyString = accuracy.normalized.toStringAsFixed(2);
      }
      children.add(
        Container(
          width: 60,
          height: 80,
          padding: const EdgeInsets.symmetric(vertical: 0, horizontal: 4),
          child: Column(
            children: [
              iconWidget,
              const SizedBox(height: 8),
              FittedBox(
                fit: BoxFit.fitWidth,
                child: Text(
                  throughputString,
                  style: const TextStyle(fontSize: 12),
                ),
              ),
              FittedBox(
                fit: BoxFit.fitWidth,
                child: Text(
                  accuracyString,
                  style: const TextStyle(fontSize: 12),
                ),
              ),
            ],
          ),
        ),
      );
    }
    return ListView(
      scrollDirection: Axis.horizontal,
      children: children,
    );
  }
}

const _matrixColorFilter = ColorFilter.matrix(<double>[
  0.2126,
  0.7152,
  0.0722,
  0,
  0,
  0.2126,
  0.7152,
  0.0722,
  0,
  0,
  0.2126,
  0.7152,
  0.0722,
  0,
  0,
  0,
  0,
  0,
  1,
  0,
]);
