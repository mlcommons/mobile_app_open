import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/app_bar_content.dart';
import 'package:mlperfbench/ui/history/history_tab.dart';
import 'package:mlperfbench/ui/history/online_tab.dart';
import 'package:mlperfbench/ui/history/utils.dart';

class ListScreen extends StatefulWidget {
  const ListScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _ListScreenState();
  }
}

class _ListScreenState extends State<ListScreen>
    with SingleTickerProviderStateMixin {
  late AppLocalizations l10n;
  late HistoryHelperUtils helper;

  late final TabController _tabController;

  AppBarContent? pushedAppBar;

  HistoryTab? history;
  OnlineTab? online;

  void triggerRebuild(void Function()? action) {
    if (!mounted) return;
    action?.call();
    setState(() {});
  }

  @override
  void initState() {
    super.initState();

    _tabController = TabController(length: 2, vsync: this);
    _tabController.addListener(() => setState(() {}));
  }

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    final state = context.watch<BenchmarkState>();
    final isOnlineEnabled = state.firebaseManager != null;

    if (isOnlineEnabled) {
      online ??= OnlineTab(
        state: state,
        triggerRebuild: triggerRebuild,
      );
    }
    history ??= HistoryTab(
      pushAppBar: (AppBarContent? appBar) {
        pushedAppBar = appBar;
        setState(() {});
      },
      state: state,
      triggerRebuild: triggerRebuild,
    );

    return WillPopScope(
      child: isOnlineEnabled ? _makeTabbedPage(context) : _makeOfflinePage(),
      onWillPop: () async {
        if (pushedAppBar == null) {
          return true;
        }
        setState(() {
          pushedAppBar!.reset();
          pushedAppBar = null;
        });
        return false;
      },
    );
  }

  Widget _makeTabbedPage(BuildContext context) {
    final buttons = _tabController.index == 0
        ? history!.getBarButtons(l10n)
        : online!.getBarButtons(l10n);
    return Scaffold(
      appBar: helper.makeAppBar(
        l10n.listScreenTitleMain,
        leading: pushedAppBar?.leading,
        actions: pushedAppBar?.trailing ?? buttons,
        bottom: pushedAppBar != null
            ? TabBar(
                tabs: const [SizedBox.shrink(), SizedBox.shrink()],
                indicatorColor: Colors.transparent,
                controller: _tabController,
              )
            : TabBar(
                tabs: [
                  Tab(text: l10n.listScreenTitleLocal),
                  Tab(text: l10n.listScreenTitleOnline),
                ],
                controller: _tabController,
              ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: [
          history!.build(context),
          online!.build(context),
        ],
        physics:
            pushedAppBar == null ? null : const NeverScrollableScrollPhysics(),
      ),
    );
  }

  Widget _makeOfflinePage() {
    return Scaffold(
      appBar: helper.makeAppBar(
        l10n.historyListTitle,
        leading: pushedAppBar?.leading,
        actions: pushedAppBar?.trailing,
      ),
      body: history!.build(context),
    );
  }
}
