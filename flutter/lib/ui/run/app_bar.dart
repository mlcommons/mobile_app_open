import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/ui/config/config_screen.dart';
import 'package:mlperfbench/ui/history/result_list_screen.dart';
import 'package:mlperfbench/ui/icons.dart';
import 'package:mlperfbench/ui/settings/settings_screen.dart';

class MyAppBar {
  static PreferredSizeWidget buildAppBar(
      String title, BuildContext context, bool addSettingsButton) {
    var actions = <Widget>[];
    if (addSettingsButton) {
      actions.add(IconButton(
        icon: const Icon(Icons.access_time),
        tooltip: 'History',
        onPressed: () {
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => const ResultListScreen()),
          );
        },
      ));
      actions.add(IconButton(
        icon: AppIcons.parameters,
        tooltip: 'Configuration',
        onPressed: () {
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => const ConfigScreen()),
          );
        },
      ));
      actions.add(IconButton(
        icon: AppIcons.settings,
        tooltip: 'Settings',
        onPressed: () {
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => const SettingsScreen()),
          );
        },
      ));
    }

    return AppBar(
      title: FittedBox(
        fit: BoxFit.fitWidth,
        child: Text(title,
            style: const TextStyle(fontSize: 24, color: AppColors.lightText)),
      ),
      centerTitle: true,
      backgroundColor: AppColors.mainScreenAppBarBackground,
      iconTheme: const IconThemeData(color: AppColors.lightAppBarIconTheme),
      actions: actions,
    );
  }
}
