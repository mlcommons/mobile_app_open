import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/result_filter.dart';
import 'package:mlperfbench/data/result_sort.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/extended_result_screen.dart';
import 'package:mlperfbench/ui/history/history_filter_screen.dart';
import 'package:mlperfbench/ui/history/history_list_item.dart';

class HistoryListScreen extends StatefulWidget {
  const HistoryListScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() => _HistoryListScreenState();
}

class _HistoryListScreenState extends State<HistoryListScreen> {
  late AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context);
    final localResults = state.resourceManager.resultManager.localResults;
    final remoteResults = state.resourceManager.resultManager.remoteResults;
    final filter = state.resourceManager.resultManager.resultFilter;
    final sort = state.resourceManager.resultManager.resultSort;
    List<ExtendedResult> results = localResults + remoteResults;
    results = results.where((result) => filter.match(result)).toList();
    results = sort.apply(results);

    final itemsList = results.map((e) => _listItem(e)).toList();

    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.menuHistory),
        actions: [
          _sortButton(sort),
          _filterButton(filter),
        ],
      ),
      body: ListView.separated(
        controller: ScrollController(),
        padding: const EdgeInsets.only(top: 20, bottom: 20),
        itemCount: itemsList.length,
        separatorBuilder: (context, index) => const Divider(),
        itemBuilder: (context, index) {
          final item = itemsList[index];
          return item.build(context);
        },
      ),
    );
  }

  PopupMenuButton<SortByEnum> _sortButton(ResultSort sort) {
    return PopupMenuButton<SortByEnum>(
      constraints: BoxConstraints(
        maxWidth: MediaQuery.of(context).size.width,
      ),
      initialValue: sort.sortBy,
      onSelected: (SortByEnum item) {
        setState(() {
          sort.sortBy = item;
        });
      },
      icon: const Icon(Icons.sort),
      itemBuilder: (context) {
        return [
          PopupMenuItem<SortByEnum>(
            value: SortByEnum.dateDesc,
            child: ListTile(
              contentPadding: const EdgeInsets.fromLTRB(4, 0, 4, 0),
              minLeadingWidth: 32,
              leading: const Icon(Icons.date_range),
              trailing: const Icon(Icons.south),
              title: Text(l10n.historySortByDate),
            ),
          ),
          PopupMenuItem<SortByEnum>(
            value: SortByEnum.dateAsc,
            child: ListTile(
              contentPadding: const EdgeInsets.fromLTRB(4, 0, 4, 0),
              minLeadingWidth: 32,
              leading: const Icon(Icons.date_range),
              trailing: const Icon(Icons.north),
              title: Text(l10n.historySortByDate),
            ),
          ),
        ];
      },
    );
  }

  Widget _filterButton(ResultFilter filter) {
    return IconButton(
      icon: Icon(
          filter.anyFilterActive ? Icons.filter_list : Icons.filter_list_off),
      tooltip: l10n.historyFilterTitle,
      onPressed: () {
        Navigator.push(
          context,
          MaterialPageRoute(
            builder: (context) => const ResultFilterScreen(),
          ),
        ).then((value) => setState(() {}));
      },
    );
  }

  HistoryListItem _listItem(ExtendedResult result) {
    return HistoryListItem(result, () {
      Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => LocalExtendedResultScreen(result: result),
        ),
      ).then((value) => setState(() {}));
    });
  }
}
