import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/data/benchmarks_data_provider.dart';
import 'package:mlperfbench/data/result_sort.dart';
import 'package:mlperfbench/data/results/benchmark_result.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/list_item.dart';
import 'package:mlperfbench/ui/history/result_filter_screen.dart';
import 'package:mlperfbench/ui/history/result_list_item.dart';
import 'package:mlperfbench/ui/history/run_details_screen.dart';

class ResultListScreen extends StatefulWidget {
  const ResultListScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _ResultListScreenState();
  }
}

class _ResultListScreenState extends State<ResultListScreen> {
  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);
    final localResults = state.resourceManager.resultManager.localResults;
    final remoteResults = state.resourceManager.resultManager.remoteResults;
    final filter = state.resourceManager.resultManager.resultFilter;
    final sort = state.resourceManager.resultManager.resultSort;
    final results = localResults + remoteResults;
    final resultsDataProvider = BenchmarksDataProvider(results);

    List<BenchmarkExportResult> resultItems =
        resultsDataProvider.resultItems(filter, sort);

    List<ListItem> itemsList = _listItems(resultItems);

    return Scaffold(
      appBar: AppBar(title: Text(l10n.menuHistory), actions: [
        PopupMenuButton<SortByEnum>(
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
                child: Text(l10n.historySortByDateDesc),
              ),
              PopupMenuItem<SortByEnum>(
                value: SortByEnum.dateAsc,
                child: Text(l10n.historySortByDateAsc),
              ),
              const PopupMenuDivider(),
              PopupMenuItem<SortByEnum>(
                value: SortByEnum.taskThroughputDesc,
                child: Text(l10n.historySortByTaskThroughputDesc),
              ),
              PopupMenuItem<SortByEnum>(
                value: SortByEnum.taskThroughputAsc,
                child: Text(l10n.historySortByTaskThroughputAsc),
              ),
            ];
          },
        ),
        IconButton(
          icon: Icon(filter.anyFilterActive
              ? Icons.filter_list
              : Icons.filter_list_off),
          tooltip: l10n.historyFilterTitle,
          onPressed: () {
            Navigator.push(
              context,
              MaterialPageRoute(
                builder: (context) => const ResultFilterScreen(),
              ),
            ).then((value) => setState(() {}));
          },
        )
      ]),
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

  List<ListItem> _listItems(List<BenchmarkExportResult> resultItems) {
    return resultItems.map((resultItem) {
      return _benchmarkListItem(resultItem);
    }).toList();
  }

  ListItem _benchmarkListItem(BenchmarkExportResult item) {
    return BenchmarkListItem(item, () {
      Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => RunDetailsScreen(result: item),
        ),
      ).then((value) => setState(() {}));
    });
  }
}
