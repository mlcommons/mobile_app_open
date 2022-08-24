import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/history_tab.dart';
import 'package:mlperfbench/ui/history/list_tab.dart';
import 'package:mlperfbench/ui/history/online_tab.dart';
import 'package:mlperfbench/ui/history/utils.dart';

class ListScreen extends StatefulWidget {
  const ListScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _ListScreenState();
  }
}

class _ListScreenState extends State<ListScreen> {
  AppBarContent? pushedAppBar;

  late HistoryTab history = HistoryTab(
    pushAppBar: (AppBarContent? appBar) {
      pushedAppBar = appBar;
      setState(() {});
    },
  );
  late OnlineTab online = OnlineTab(
    pushAppBar: (AppBarContent? appBar) {
      pushedAppBar = appBar;
      setState(() {});
    },
  );

  @override
  Widget build(BuildContext context) {
    HistoryHelperUtils helper;
    final l10n = AppLocalizations.of(context);
    helper = HistoryHelperUtils(l10n);

    return WillPopScope(
      child: DefaultTabController(
        length: 2,
        child: Builder(builder: (BuildContext context) {
          // final index = DefaultTabController.of(context)!.index;
          return Scaffold(
            appBar: helper.makeAppBar(
              'Results',
              leading: pushedAppBar?.leading,
              actions: pushedAppBar?.trailing,
              bottom: pushedAppBar != null
                  ? const TabBar(
                      tabs: [SizedBox.shrink(), SizedBox.shrink()],
                      indicatorColor: Colors.transparent,
                    )
                  : TabBar(
                      tabs: [
                        Tab(text: history.getTabName()),
                        Tab(text: online.getTabName()),
                      ],
                    ),
            ),
            body: TabBarView(
              children: [
                history,
                online,
              ],
              physics: pushedAppBar == null
                  ? null
                  : const NeverScrollableScrollPhysics(),
            ),
          );
        }),
      ),
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
}
