import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/ui/history/history_screen.dart';
import 'package:mlperfbench/ui/icons.dart';
import '../config/config_screen.dart';
import '../settings/settings_screen.dart';

class MyAppBar {
  static PreferredSizeWidget buildAppBar(
      String title, BuildContext context, bool addSettingsButton) {
    var actions = <Widget>[];
    if (addSettingsButton) {
      actions.add(IconButton(
        icon: Icon(Icons.list),
        tooltip: 'History',
        onPressed: () {
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => HistoryScreen()),
          );
        },
      ));
      actions.add(IconButton(
        icon: AppIcons.parameters,
        tooltip: 'Configuration',
        onPressed: () {
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => ConfigScreen()),
          );
        },
      ));
      actions.add(IconButton(
        icon: AppIcons.settings,
        tooltip: 'Settings',
        onPressed: () {
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => SettingsScreen()),
          );
        },
      ));
    }

    return AppBar(
      title: FittedBox(
        fit: BoxFit.fitWidth,
        child: Text(title,
            style: TextStyle(fontSize: 24, color: AppColors.lightText)),
      ),
      centerTitle: true,
      backgroundColor: AppColors.darkAppBarBackground,
      iconTheme: IconThemeData(color: AppColors.lightAppBarIconTheme),
      actions: actions,
    );
  }
}
