import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/confirm_dialog.dart';
import 'package:mlperfbench/ui/history/app_bar_content.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'result_details_screen.dart';

class HistoryTab extends StatefulWidget {
  final void Function(AppBarContent? appBar) pushAppBar;

  HistoryTab({
    Key? key,
    required this.pushAppBar,
  }) : super(key: key);

  @override
  _HistoryTab createState() => _HistoryTab();

  void Function() enableSelection = () {};

  List<Widget>? getBarButtons(AppLocalizations l10n) {
    final enableSelectionButton = IconButton(
      icon: const Icon(Icons.check_box_outlined),
      tooltip: l10n.historyListSelectionEnable,
      onPressed: () => enableSelection(),
    );
    return [enableSelectionButton];
  }
}

class _HistoryTab extends State<HistoryTab>
    with AutomaticKeepAliveClientMixin<HistoryTab> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  late List<ExtendedResult> itemList;

  late BenchmarkState state;

  bool isSelectionMode = false;
  List<bool>? selected;
  bool isSelectAll = false;

  @override
  void initState() {
    super.initState();

    widget.enableSelection = enableSelection;
  }

  void resetSelection(bool value) {
    selected = List<bool>.generate(itemList.length, (_) => value);
  }

  late final cancelSelection = IconButton(
    icon: const Icon(Icons.close),
    tooltip: l10n.historyListSelectionCancel,
    onPressed: () => setState(() => disableSelectionMode()),
  );

  late final selectAll = IconButton(
    icon: Icon(isSelectAll ? Icons.deselect : Icons.select_all),
    tooltip: isSelectAll
        ? l10n.historyListSelectionDeselect
        : l10n.historyListSelectionSelectAll,
    onPressed: () {
      isSelectAll = !isSelectAll;
      setState(() => resetSelection(isSelectAll));
    },
  );

  late final delete = IconButton(
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

  void enableSelection() {
    isSelectionMode = true;
    widget.pushAppBar(AppBarContent(
      leading: cancelSelection,
      trailing: [delete, selectAll],
      reset: () => setState(() => disableSelectionMode()),
    ));
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);

    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    state = context.watch<BenchmarkState>();

    itemList = state.resourceManager.resultManager.results;
    if (selected == null) {
      resetSelection(false);
    }

    return ListView.separated(
      controller: ScrollController(),
      padding: const EdgeInsets.only(top: 20),
      itemCount: itemList.length,
      separatorBuilder: (context, index) => const Divider(),
      itemBuilder: (context, index) {
        final uiIndex = itemList.length - index - 1;
        return _makeItem(context, uiIndex);
      },
    );
  }

  void disableSelectionMode() {
    isSelectionMode = false;
    isSelectAll = false;
    selected = null;
    widget.pushAppBar(null);
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
              onChanged: (bool? x) => _toggleSelection(index),
            )
          : null,
      onTap: isSelectionMode
          ? () => _toggleSelection(index)
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
          : () => setState(() {
                enableSelection();
                _toggleSelection(index);
              }),
    );
  }

  @override
  bool get wantKeepAlive => true;
}
