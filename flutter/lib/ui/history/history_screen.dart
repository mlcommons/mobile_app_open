import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/error_dialog.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'result_details_screen.dart';

class HistoryScreen extends StatefulWidget {
  const HistoryScreen({Key? key}) : super(key: key);

  @override
  _HistoryScreen createState() => _HistoryScreen();
}

class _HistoryScreen extends State<HistoryScreen> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  bool isSelectionMode = false;
  List<bool>? selected;
  bool isSelectAll = false;

  void resetSelection(int listLength, bool value) {
    selected = List<bool>.generate(listLength, (_) => value);
  }

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    final state = context.watch<BenchmarkState>();

    final results = state.resourceManager.resultManager.results;
    final length = results.length;
    if (selected == null) {
      resetSelection(length, false);
    }

    final cancelSelection = IconButton(
      icon: const Icon(Icons.close),
      tooltip: l10n.historyListSelectionCancel,
      onPressed: () {
        setState(() {
          disableSelectionMode();
        });
      },
    );
    final enableSelection = IconButton(
      icon: const Icon(Icons.done),
      tooltip: l10n.historyListSelectionEnable,
      onPressed: () {
        setState(() {
          isSelectionMode = true;
        });
      },
    );
    final selectAll = IconButton(
      icon: Icon(isSelectAll ? Icons.deselect : Icons.select_all),
      tooltip: isSelectAll
          ? l10n.historyListSelectionDeselect
          : l10n.historyListSelectionSelectAll,
      onPressed: () {
        isSelectAll = !isSelectAll;
        setState(() {
          resetSelection(length, isSelectAll);
        });
      },
    );
    final delete = IconButton(
      icon: const Icon(Icons.delete),
      tooltip: l10n.historyListSelectionDelete,
      onPressed: () async {
        final dialogResult = await showConfirmDialog(
          context,
          l10n.historyListSelectionDeleteConfirm,
          title: l10n.historyListSelectionDelete,
        );
        switch (dialogResult) {
          case ConfirmDialogAction.ok:
            setState(() {
              state.resourceManager.resultManager.removeSelected(selected!);
              disableSelectionMode();
            });
            break;
          case null:
          case ConfirmDialogAction.cancel:
            break;
        }
      },
    );
    return Scaffold(
      appBar: helper.makeAppBar(
        l10n.historyListTitle,
        leading: isSelectionMode ? cancelSelection : null,
        actions: <Widget>[
          if (!isSelectionMode) enableSelection,
          if (isSelectionMode) selectAll,
          if (isSelectionMode) delete,
        ],
      ),
      body: ListView.separated(
        padding: const EdgeInsets.only(top: 20),
        itemCount: results.length,
        separatorBuilder: (context, index) => const Divider(),
        itemBuilder: (context, index) {
          final uiIndex = results.length - index - 1;
          return _makeItem(context, results, uiIndex);
        },
      ),
    );
  }

  void disableSelectionMode() {
    isSelectionMode = false;
    isSelectAll = false;
    selected = null;
  }

  void _toggleSelection(int index) {
    if (isSelectionMode) {
      setState(() {
        selected![index] = !selected![index];
      });
    }
  }

  Widget _makeItem(
    BuildContext context,
    List<ExtendedResult> allItems,
    int index,
  ) {
    final results = allItems[index].results;
    final firstRunInfo = results.list.first;
    final startDatetime = firstRunInfo.performance?.startDatetime ??
        firstRunInfo.accuracy!.startDatetime;
    bool isSelected = selected![index];

    final qps = results.calculateAverageThroughput().toStringAsFixed(2);
    final benchmarksNum = results.list.length.toString();

    return ListTile(
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(
          helper.formatDate(startDatetime.toLocal()),
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
      ),
      subtitle: Text(
        l10n.historyListElementSubtitle
            .replaceFirst('<throughput>', qps)
            .replaceFirst('<benchmarks#>', benchmarksNum),
        style: const TextStyle(fontWeight: FontWeight.bold),
      ),
      trailing: isSelectionMode
          ? Checkbox(
              value: isSelected,
              onChanged: (bool? x) => _toggleSelection(index),
            )
          : const Icon(Icons.chevron_right),
      onTap: isSelectionMode
          ? () => _toggleSelection(index)
          : () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailsScreen(result: allItems[index]),
                ),
              );
            },
      onLongPress: () {
        if (!isSelectionMode) {
          setState(() {
            isSelectionMode = true;
            _toggleSelection(index);
          });
        }
      },
    );
  }
}
