import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/result_filter.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/result_details_screen.dart';
import 'package:mlperfbench/ui/history/result_filter_screen.dart';
import 'package:mlperfbench/ui/history/run_details_screen.dart';
import 'package:mlperfbench/ui/history/utils.dart';

class ResultListScreen extends StatefulWidget {
  const ResultListScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _ResultListScreenState();
  }
}

abstract class ListItem {
  Widget build(BuildContext context);
}

class ExtendedResultListItem implements ListItem {
  final ExtendedResult item;
  final void Function()? tapHandler;

  ExtendedResultListItem(this.item, this.tapHandler);

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final helper = HistoryHelperUtils(l10n);

    return ListTile(
      title: Text(
        helper.formatDate(item.meta.creationDate.toLocal()),
        style: const TextStyle(fontWeight: FontWeight.bold),
      ),
      subtitle: Text(item.meta.uuid),
      onTap: tapHandler,
    );
  }
}

class BenchmarkListItem implements ListItem {
  final BenchmarkExportResult item;
  final void Function()? tapHandler;

  BenchmarkListItem(this.item, this.tapHandler);

  @override
  Widget build(BuildContext context) => ListTile(
        title: Text(
          item.benchmarkId,
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
        subtitle: Text(item.performanceRun!.throughput!.value.toString()),
        onTap: tapHandler,
      );
}

class _ResultListScreenState extends State<ResultListScreen> {
  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);
    final results = state.resourceManager.resultManager.results;
    final filter = state.resourceManager.resultManager.resultFilter;

    List<ListItem> itemList = listItems(results, filter);

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
        itemCount: itemList.length,
        separatorBuilder: (context, index) => const Divider(),
        itemBuilder: (context, index) {
          final item = itemList[index];
          return item.build(context);
        },
      ),
    );
  }

  List<ListItem> listItems(List<ExtendedResult> items, ResultFilter filter) {
    // Filter and sort the items based on the selected filter option
    List<ExtendedResult> filteredItems = List<ExtendedResult>.from(items);

    if (filter.sortBy == SortBy.taskThroughputDesc) {
      return _benchmarkResults(filteredItems, filter);
    }
    return _extendedResults(filteredItems, filter);
  }

  List<ListItem> _benchmarkResults(
      List<ExtendedResult> items, ResultFilter filter) {
    List<BenchmarkExportResult> benchmarks = items
        .expand((item) => item.results)
        .where((element) =>
            element.performanceRun != null &&
            element.performanceRun!.throughput != null)
        .toList();

    benchmarks.sort((a, b) =>
        b.performanceRun!.throughput!.compareTo(a.performanceRun!.throughput!));

    return benchmarks
        .map((benchmark) => BenchmarkListItem(benchmark, () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => RunDetailsScreen(result: benchmark),
                ),
              ).then((value) => setState(() {}));
            }))
        .toList();
  }

  List<ListItem> _extendedResults(
      List<ExtendedResult> items, ResultFilter filter) {
    if (filter.sortBy == SortBy.dateAsc) {
      items.sort((a, b) => a.meta.creationDate.compareTo(b.meta.creationDate));
    } else if (filter.sortBy == SortBy.dateDesc) {
      items.sort((a, b) => b.meta.creationDate.compareTo(a.meta.creationDate));
    }
    return items
        .map((item) => ExtendedResultListItem(item, () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailsScreen(result: item),
                ),
              ).then((value) => setState(() {}));
            }))
        .toList();
  }
}
