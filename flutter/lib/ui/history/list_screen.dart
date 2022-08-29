import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/app_bar_content.dart';
import 'package:mlperfbench/ui/history/history_tab.dart';
import 'package:mlperfbench/ui/history/online_tab.dart';
import 'package:mlperfbench/ui/history/utils.dart';
import 'tab_interface.dart';

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
  final List<TabInterface> tabs = [];

  AppBarContent? pushedAppBar;

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

    if (tabs.isEmpty) {
      tabs.add(HistoryTab(
        pushAppBar: (AppBarContent? appBar) {
          pushedAppBar = appBar;
          triggerRebuild(null);
        },
        state: state,
        triggerRebuild: triggerRebuild,
      ));
      if (isOnlineEnabled) {
        tabs.add(OnlineTab(
          state: state,
          triggerRebuild: triggerRebuild,
        ));
      }
    }

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
    return Scaffold(
      appBar: helper.makeAppBar(
        l10n.listScreenTitleMain,
        leading: pushedAppBar?.leading,
        actions: pushedAppBar?.trailing ??
            tabs[_tabController.index].getBarButtons(l10n),
        bottom: pushedAppBar != null
            ? TabBar(
                tabs: const [SizedBox.shrink(), SizedBox.shrink()],
                indicatorColor: Colors.transparent,
                controller: _tabController,
              )
            : TabBar(
                tabs: tabs.map((e) => Tab(text: e.getTabName(l10n))).toList(),
                controller: _tabController,
              ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: tabs.map((e) => e.build(context)).toList(),
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
      body: tabs.first.build(context),
    );
  }
}
