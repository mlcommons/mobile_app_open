import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart' hide Icons;

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/icons.dart';
import 'config_screen.dart';
import 'settings_screen.dart';

class MyAppBar {
  static PreferredSizeWidget buildAppBar(
      String title, BuildContext context, bool addSettingsButton) {
    var actions = <Widget>[];
    if (addSettingsButton) {
      actions.add(IconButton(
        icon: Icons.parameters,
        tooltip: 'Configuration',
        onPressed: () {
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => ConfigScreen()),
          );
        },
      ));
      actions.add(IconButton(
        icon: Icons.settings,
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
        child: Text(title, style: TextStyle(fontSize: 24, color: Colors.white)),
      ),
      centerTitle: true,
      backgroundColor:
          OFFICIAL_BUILD ? Color(0xFF31A3E2) : Colors.brown.shade400,
      iconTheme: IconThemeData(color: Colors.white),
      actions: actions,
    );
  }
}
