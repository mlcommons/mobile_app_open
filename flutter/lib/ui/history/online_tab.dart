import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/firebase/cache_helper.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/list_tab.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'result_details_screen.dart';

class OnlineTab extends StatefulWidget {
  final void Function(AppBarContent? appBar) pushAppBar;

  OnlineTab({
    Key? key,
    required this.pushAppBar,
  }) : super(key: key) {
    print('OnlineTab ctor');
  }

  @override
  _OnlineTab createState() {
    return _OnlineTab();
  }

  String getTabName() {
    return 'online';
  }
}

class _OnlineTab extends State<OnlineTab>
    with AutomaticKeepAliveClientMixin<OnlineTab> {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  final List<ExtendedResult> itemList = [];

  late BenchmarkState state;

  FirebaseCacheHelper? cacheHelper;
  String currentStartUuid = '';
  bool isFetching = false;
  String error = '';

  _OnlineTab() {
    print('OnlineTab state ctor');
  }

  @override
  void initState() {
    super.initState();
  }

  Future<void> fetchData() async {
    isFetching = true;
    try {
      await cacheHelper!.fetchBatch(from: currentStartUuid);
    } catch (e, t) {
      print(e);
      print(t);
    }
    isFetching = false;
    if (mounted) {
      setState(() {});
    }
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);

    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    state = context.watch<BenchmarkState>();

    final fm = state.firebaseManager;
    if (fm == null) {
      // TODO remove online tab when online is disabled
      return Center(
        child: Text('Online results are unavailable'),
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
        child: Text('Online results are empty'),
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

  @override
  bool get wantKeepAlive => true;
}
