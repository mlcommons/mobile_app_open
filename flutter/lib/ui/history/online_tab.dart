import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/firebase/cache_helper.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/filter/filter_screen.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'filter/filter_terms.dart';
import 'result_details_screen.dart';
import 'tab_interface.dart';

class OnlineTab implements TabInterface {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  final BenchmarkState state;
  final void Function([void Function()? action]) triggerRebuild;

  final List<ExtendedResult> itemList = [];

  FirebaseCacheHelper? cacheHelper;
  String currentStartUuid = '';
  bool isFetching = false;
  String error = '';
  FilterTerms? filterTerms;

  OnlineTab({
    required this.state,
    required this.triggerRebuild,
  });

  @override
  String getTabName(AppLocalizations l10n) {
    return l10n.listScreenTitleOnline;
  }

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    final fm = state.firebaseManager;
    if (fm == null) {
      // this widget should never be displayed without access to firebase
      // but let's handle this case gracefully
      return Center(
        child: Text(l10n.listScreenOnlineDisabled),
      );
    }

    cacheHelper ??= FirebaseCacheHelper(fm.restHelper);

    final currentBatch = cacheHelper!.getBatch(from: currentStartUuid);
    if (currentBatch == null) {
      fetchData();
    }

    if (isFetching) {
      return const Center(
        child: CircularProgressIndicator(),
      );
    }

    itemList.clear();
    for (var uuid in currentBatch!) {
      itemList.add(cacheHelper!.getByUuid(uuid)!);
    }

    if (itemList.isEmpty) {
      return Center(
        child: Text(l10n.listScreenNoResultsFound),
      );
    }

    return ListView.separated(
      controller: ScrollController(),
      padding: const EdgeInsets.only(top: 20),
      itemCount: itemList.length,
      separatorBuilder: (context, index) => const Divider(),
      itemBuilder: (context, index) {
        final uiIndex = itemList.length - index - 1;
        return _makeItem(context, uiIndex);
      },
    );
  }

  @override
  List<Widget>? getBarButtons(BuildContext context, AppLocalizations l10n) {
    final filterButton = IconButton(
      icon: const Icon(Icons.filter_alt),
      tooltip: l10n.filterTitle,
      onPressed: () async {
        filterTerms ??= FilterTerms.create(l10n);
        await Navigator.push(
          context,
          MaterialPageRoute<void>(
            builder: (BuildContext context) => Provider.value(
              value: filterTerms,
              child: const FilterScreen(),
            ),
          ),
        );
        cacheHelper!.reset();
        fetchData();
        triggerRebuild();
      },
    );
    return [filterButton];
  }

  Future<void> fetchData() async {
    isFetching = true;
    try {
      var excludedOsList = <String>[];
      final filterTerms = this.filterTerms;
      if (filterTerms != null) {
        if (filterTerms.osOptions.any((e) => e.value)) {
          excludedOsList = filterTerms.osOptions
              .where((e) => !e.value)
              .map((e) => e.key)
              .toList();
        }
      }
      await cacheHelper!.fetchBatch(
        from: currentStartUuid,
        excludedOsList: excludedOsList,
      );
    } catch (e, t) {
      print(e);
      print(t);
    } finally {
      isFetching = false;
    }
    triggerRebuild();
  }

  Widget _makeItem(
    BuildContext context,
    int index,
  ) {
    final item = itemList[index];
    final results = item.results;
    final firstRunInfo = results.list.first;
    final startDatetime = firstRunInfo.performance?.startDatetime ??
        firstRunInfo.accuracy!.startDatetime;

    return helper.makeListItem(
      title: helper.formatDate(startDatetime.toLocal()),
      specialTitleColor: results.list
          .any((runRes) => !(runRes.performance?.loadgenValidity ?? false)),
      subtitle: item.envInfo.model,
      onTap: () {
        Navigator.push(
          context,
          MaterialPageRoute(
            builder: (context) => DetailsScreen(result: item),
          ),
        );
      },
    );
  }
}
