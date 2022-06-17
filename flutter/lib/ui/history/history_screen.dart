import 'package:flutter/material.dart';

import 'package:intl/intl.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'result_details_screen.dart';

class HistoryScreen extends StatefulWidget {
  @override
  _HistoryScreen createState() => _HistoryScreen();
}

class _HistoryScreen extends State<HistoryScreen> {
  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);

    final results = state.resourceManager.resultManager.results;
    var dateFormat = DateFormat('yyyy-MM-dd HH:mm');

    return Scaffold(
      appBar: AppBar(
        title: Text(
          l10n.historyListTitle,
          style: TextStyle(fontSize: 24, color: AppColors.lightText),
        ),
        centerTitle: true,
        backgroundColor: AppColors.darkAppBarBackground,
        iconTheme: IconThemeData(color: AppColors.lightAppBarIconTheme),
      ),
      body: ListView.separated(
        padding: const EdgeInsets.only(top: 20),
        itemCount: results.length,
        separatorBuilder: (context, index) {
          return Divider();
        },
        itemBuilder: (context, index) {
          final result = results[results.length - index - 1];
          final firstRunInfo = result.results.list.first;
          final startDatetime = firstRunInfo.performance?.startDatetime ??
              firstRunInfo.accuracy!.startDatetime;
          return ListTile(
            title: Padding(
              padding: const EdgeInsets.only(bottom: 5),
              child: Text(
                dateFormat.format(startDatetime.toLocal()),
                style: TextStyle(fontWeight: FontWeight.bold),
              ),
            ),
            subtitle: Text(
              l10n.historyListElementSubtitle
                  .replaceFirst(
                      '<throughput>',
                      result.results
                          .calculateAverageThroughput()
                          .toStringAsFixed(2))
                  .replaceFirst(
                      '<benchmarks#>', result.results.list.length.toString()),
              style: TextStyle(fontWeight: FontWeight.bold),
            ),
            trailing: Icon(Icons.arrow_forward_ios),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailsScreen(result: result),
                ),
              );
            },
          );
        },
      ),
    );
  }
}
