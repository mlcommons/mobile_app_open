import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'result_details_screen.dart';

class HistoryScreen extends StatefulWidget {
  @override
  _HistoryScreen createState() => _HistoryScreen();
}

class _HistoryScreen extends State<HistoryScreen> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    final state = context.watch<BenchmarkState>();

    final results = state.resourceManager.resultManager.results;

    return Scaffold(
      appBar: helper.makeAppBar(l10n.historyListTitle),
      body: ListView.separated(
        padding: const EdgeInsets.only(top: 20),
        itemCount: results.length,
        separatorBuilder: (context, index) => Divider(),
        itemBuilder: (context, index) {
          return _makeItem(context, results[results.length - index - 1]);
        },
      ),
    );
  }

  Widget _makeItem(BuildContext context, ExtendedResult extendedResult) {
    final results = extendedResult.results;
    final firstRunInfo = results.list.first;
    final startDatetime = firstRunInfo.performance?.startDatetime ??
        firstRunInfo.accuracy!.startDatetime;

    final qps = results.calculateAverageThroughput().toStringAsFixed(2);
    final benchmarksNum = results.list.length.toString();

    return ListTile(
      title: Padding(
        padding: const EdgeInsets.only(bottom: 5),
        child: Text(
          helper.formatDate(startDatetime.toLocal()),
          style: TextStyle(fontWeight: FontWeight.bold),
        ),
      ),
      subtitle: Text(
        l10n.historyListElementSubtitle
            .replaceFirst('<throughput>', qps)
            .replaceFirst('<benchmarks#>', benchmarksNum),
        style: TextStyle(fontWeight: FontWeight.bold),
      ),
      trailing: Icon(Icons.chevron_right),
      onTap: () {
        Navigator.push(
          context,
          MaterialPageRoute(
            builder: (context) => DetailsScreen(result: extendedResult),
          ),
        );
      },
    );
  }
}
