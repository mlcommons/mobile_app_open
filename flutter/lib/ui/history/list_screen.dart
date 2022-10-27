import 'package:flutter/material.dart';

import 'package:mlperfbench_common/firebase/manager.dart';
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

  void triggerRebuild([void Function()? action]) {
    if (!mounted) return;
    action?.call();
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    initTabs(context);

    return WillPopScope(
      child: tabs.length > 1 ? _makeTabbedPage(context) : _makeSingleTabPage(),
      onWillPop: () async => !cancelAction(),
    );
  }

  void initTabs(BuildContext context) {
    if (tabs.isNotEmpty) return;

    final state = context.watch<BenchmarkState>();
    final fm = context.watch<FirebaseManager?>();

    tabs.add(HistoryTab(
      pushAction: pushAction,
      resultManager: state.resourceManager.resultManager,
      triggerRebuild: triggerRebuild,
    ));

    final isOnlineEnabled = fm != null;
    if (isOnlineEnabled) {
      tabs.add(OnlineTab(
        triggerRebuild: triggerRebuild,
      ));
    }

    _tabController = TabController(length: tabs.length, vsync: this);
    _tabController.addListener(() => setState(() {}));
  }

  Widget _makeTabbedPage(BuildContext context) {
    return Scaffold(
      appBar: helper.makeAppBar(
        l10n.listScreenTitleMain,
        leading: _makeCancelButton(),
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
        physics:
            pushedAppBar == null ? null : const NeverScrollableScrollPhysics(),
        children: tabs.map((e) => e.build(context)).toList(),
      ),
    );
  }

  Widget _makeSingleTabPage() {
    return Scaffold(
      appBar: helper.makeAppBar(
        l10n.historyListTitle,
        leading: _makeCancelButton(),
        actions: pushedAppBar?.trailing,
      ),
      body: tabs.first.build(context),
    );
  }

  Widget? _makeCancelButton() {
    if (pushedAppBar == null) return null;
    return IconButton(
      icon: const Icon(Icons.close),
      tooltip: l10n.historyListSelectionCancel,
      onPressed: cancelAction,
    );
  }

  void pushAction(AppBarContent? appBarContent) {
    pushedAppBar = appBarContent;
    triggerRebuild();
  }

  /// returns true if action was cancelled, false if there was no action
  bool cancelAction() {
    if (pushedAppBar == null) return false;

    pushedAppBar!.reset();
    pushedAppBar = null;
    triggerRebuild();

    return true;
  }
}
