import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/history/app_bar_content.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'result_details_screen.dart';
import 'tab_interface.dart';

class HistoryTab implements TabInterface {
  final void Function(AppBarContent? appBar) pushAction;
  final void Function([void Function() action]) triggerRebuild;
  final BenchmarkState state;

  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  late List<ExtendedResult> itemList;
  bool isSelectionMode = false;
  List<bool>? selected;
  bool isSelectAll = false;

  HistoryTab({
    required this.pushAction,
    required this.triggerRebuild,
    required this.state,
  });

  @override
  String getTabName(AppLocalizations l10n) {
    return l10n.listScreenTitleLocal;
  }

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    itemList = state.resourceManager.resultManager.results;
    if (selected == null) {
      resetSelection(false);
    }

    delete = _makeDeleteButton(context);

    return ListView.separated(
      controller: ScrollController(),
      padding: const EdgeInsets.only(top: 20),
      itemCount: itemList.length,
      separatorBuilder: (context, index) => const Divider(),
      itemBuilder: (context, index) {
        final uiIndex = itemList.length - index - 1;
        return _makeListItem(context, uiIndex);
      },
    );
  }

  @override
  List<Widget>? getBarButtons(AppLocalizations l10n) {
    final enableSelectionButton = IconButton(
      icon: const Icon(Icons.check_box_outlined),
      tooltip: l10n.historyListSelectionEnable,
      onPressed: enableSelection,
    );
    return [enableSelectionButton];
  }

  void resetSelection(bool value) {
    selected = List<bool>.generate(itemList.length, (_) => value);
  }

  late final selectAll = IconButton(
    icon: Icon(isSelectAll ? Icons.deselect : Icons.select_all),
    tooltip: isSelectAll
        ? l10n.historyListSelectionDeselect
        : l10n.historyListSelectionSelectAll,
    onPressed: () {
      isSelectAll = !isSelectAll;
      triggerRebuild(() => resetSelection(isSelectAll));
    },
  );

  late Widget delete;
  Widget _makeDeleteButton(BuildContext context) => IconButton(
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
              triggerRebuild(() {
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

  void enableSelection() {
    isSelectionMode = true;
    pushAction(AppBarContent(
      trailing: [delete, selectAll],
      reset: () => triggerRebuild(() => disableSelectionMode()),
    ));
  }

  void disableSelectionMode() {
    isSelectionMode = false;
    isSelectAll = false;
    selected = null;
    pushAction(null);
  }

  void toggleSelectionForItem(int index) {
    if (isSelectionMode) {
      selected![index] = !selected![index];
      triggerRebuild();
    }
  }

  Widget _makeListItem(
    BuildContext context,
    int index,
  ) {
    final item = itemList[index];
    final results = item.results;
    final firstRunInfo = results.list.first;
    final startDatetime = firstRunInfo.performance?.startDatetime ??
        firstRunInfo.accuracy!.startDatetime;
    bool isSelected = selected![index];

    return helper.makeListItem(
      title: helper.formatDate(startDatetime.toLocal()),
      specialTitleColor: results.list
          .any((runRes) => !(runRes.performance?.loadgenValidity ?? false)),
      trailing: isSelectionMode
          ? Checkbox(
              value: isSelected,
              onChanged: (bool? x) => toggleSelectionForItem(index),
            )
          : null,
      onTap: isSelectionMode
          ? () => toggleSelectionForItem(index)
          : () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailsScreen(result: item),
                ),
              );
            },
      onLongPress: isSelectionMode
          ? null
          : () => triggerRebuild(() {
                enableSelection();
                toggleSelectionForItem(index);
              }),
    );
  }
}
