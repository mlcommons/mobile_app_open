import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/result_details_screen.dart';
import 'package:mlperfbench/ui/history/result_filter_screen.dart';
import 'package:mlperfbench/ui/history/result_item.dart';
import 'package:mlperfbench/ui/history/result_list_item.dart';
import 'package:mlperfbench/ui/history/results_data_provider.dart';
import 'package:mlperfbench/ui/history/run_details_screen.dart';
import 'list_item.dart';

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
    final results = state.resourceManager.resultManager.results;
    final filter = state.resourceManager.resultManager.resultFilter;
    final resultsDataProvider = ResultsDataProvider(results, filter);

    List<ResultItem> resultItems = resultsDataProvider.resultItems();

    List<ListItem> itemsList = _listItems(resultItems);

    return Scaffold(
      appBar: AppBar(title: Text(l10n.historyListTitle), actions: [
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
        padding: const EdgeInsets.only(top: 20),
        itemCount: itemsList.length,
        separatorBuilder: (context, index) => const Divider(),
        itemBuilder: (context, index) {
          final item = itemsList[index];
          return item.build(context);
        },
      ),
    );
  }

  List<ListItem> _listItems(List<ResultItem> resultItems) {
    return resultItems.map((resultItem) {
      if (resultItem.item is BenchmarkExportResult) {
        return _benchmarkListItem(resultItem.item as BenchmarkExportResult);
      }
      return _resultListItem(resultItem.item as ExtendedResult);
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

  ListItem _resultListItem(ExtendedResult item) {
    return ExtendedResultListItem(item, () {
      Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => DetailsScreen(result: item),
        ),
      ).then((value) => setState(() {}));
    });
  }
}
