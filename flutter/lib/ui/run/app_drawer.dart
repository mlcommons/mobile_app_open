import 'package:flutter/material.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/config/config_screen.dart';
import 'package:mlperfbench/ui/history/result_list_screen.dart';
import 'package:mlperfbench/ui/settings/about_screen.dart';
import 'package:mlperfbench/ui/settings/settings_screen.dart';

class AppDrawer extends StatelessWidget {
  const AppDrawer({super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    return Drawer(
      // Add a ListView to the drawer. This ensures the user can scroll
      // through the options in the drawer if there isn't enough vertical
      // space to fit everything.
      child: ListView(
        // Important: Remove any padding from the ListView.
        padding: EdgeInsets.zero,
        children: [
          const SizedBox(
            height: 80.0,
            child: DrawerHeader(
              // decoration: BoxDecoration(color: Colors.black),
              // margin: EdgeInsets.all(0.0),
              // padding: EdgeInsets.all(0.0),

              child: Text('MLPerf Mobile'),
            ),
          ),
          ListTile(
            leading: const Icon(Icons.access_time),
            title: Text(l10n.menuHistory),
            onTap: () {
              Navigator.pop(context);
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => const ResultListScreen(),
                ),
              );
            },
          ),
          ListTile(
            leading: const Icon(Icons.tune),
            title: Text(l10n.menuBenchmarkConfiguration),
            onTap: () {
              Navigator.pop(context);
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => const ConfigScreen(),
                ),
              );
            },
          ),
          ListTile(
            leading: const Icon(Icons.settings),
            title: Text(l10n.menuSettings),
            onTap: () {
              Navigator.pop(context);
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => const SettingsScreen(),
                ),
              );
            },
          ),
          ListTile(
            leading: const Icon(Icons.info),
            title: Text(l10n.menuAbout),
            onTap: () {
              Navigator.pop(context);
              Navigator.push(
                  context,
                  MaterialPageRoute(
                    builder: (context) => const AboutScreen(),
                  ));
            },
          ),
        ],
      ),
    );
  }
}
