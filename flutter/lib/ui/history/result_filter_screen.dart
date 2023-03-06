import 'package:flutter/material.dart';
import 'package:mlperfbench/ui/history/utils.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class ResultFilterScreen extends StatefulWidget {
  const ResultFilterScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _ResultFilterScreenState();
  }
}

class _ResultFilterScreenState extends State<ResultFilterScreen> {
  @override
  Widget build(BuildContext context) {
    final state = context.watch<BenchmarkState>();
    final l10n = AppLocalizations.of(context);
    final helper = HistoryHelperUtils(l10n);
    return Scaffold(
        appBar: helper.makeAppBar('Filter'),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.start,
            children: [
              const Text('Some filter here'),
              ElevatedButton(
                onPressed: () {
                  state.resourceManager.resultManager.filteredResults = state
                      .resourceManager.resultManager.results
                      .take(1)
                      .toList();
                  Navigator.pop(context);
                },
                child: const Text('Take 1'),
              ),
              ElevatedButton(
                onPressed: () {
                  state.resourceManager.resultManager.filteredResults = state
                      .resourceManager.resultManager.results
                      .take(2)
                      .toList();
                  Navigator.pop(context);
                },
                child: const Text('Take 2'),
              )
            ],
          ),
        ));
  }
}
