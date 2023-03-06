import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/result_filter.dart';
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
    itemList =
        results.where((e) => ResultFilter.from(e).match(filter)).toList();

    return Scaffold(
      appBar: helper.makeAppBar(l10n.historyListTitle, actions: [
        IconButton(
          icon: const Icon(Icons.filter_list),
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
            title: Text(helper.formatDate(item.meta.creationDate.toLocal())),
            subtitle: Text(helper.makeModelDescription(item.environmentInfo)),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailsScreen(result: item),
                ),
              );
            },
          );
        },
      ),
    );
  }
}
