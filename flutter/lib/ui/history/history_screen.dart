import 'package:flutter/material.dart';

import 'package:intl/intl.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/ui/history/result_details_screen.dart';

// import 'package:mlperfbench/localizations/app_localizations.dart';

class HistoryScreen extends StatefulWidget {
  @override
  _HistoryScreen createState() => _HistoryScreen();
}

class _HistoryScreen extends State<HistoryScreen> {
  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    // final stringResources = AppLocalizations.of(context);

    final results = state.resourceManager.resultManager.results;
    var dateFormat = DateFormat('yyyy-MM-dd HH:mm');

    return Scaffold(
      appBar: AppBar(
        title: Text(
          'Past results', // TODO move to resources
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
              'Average: ${result.results.calculateAverageThroughput().toStringAsFixed(2)}',
              style: TextStyle(fontWeight: FontWeight.bold),
            ),
            trailing: Icon(Icons.arrow_forward_ios),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailsScreen(
                    result: result,
                  ),
                ),
              );
            },
          );
        },
      ),
    );
  }
}
