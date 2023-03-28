import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/result_details_screen.dart';
import 'package:mlperfbench/ui/history/result_filter_screen.dart';
import 'package:mlperfbench/ui/history/utils.dart';

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
    final helper = HistoryHelperUtils(l10n);
    final results = state.resourceManager.resultManager.results;
    final filter = state.resourceManager.resultManager.resultFilter;
    List<ExtendedResult> itemList = results;
    itemList = results.where((result) => filter.match(result)).toList();
    itemList.sort((a, b) => a.meta.creationDate.compareTo(b.meta.creationDate));

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
          return ListTile(
            title: Text(
              helper.formatDate(item.meta.creationDate.toLocal()),
              style: const TextStyle(fontWeight: FontWeight.bold),
            ),
            subtitle: Text(item.meta.uuid),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailsScreen(result: item),
                ),
              ).then((value) => setState(() {}));
            },
          );
        },
      ),
    );
  }
}
