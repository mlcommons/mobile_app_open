import 'package:flutter/material.dart';

import 'package:url_launcher/url_launcher.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';
import 'package:mlperfbench/ui/history/history_list_screen.dart';
import 'package:mlperfbench/ui/home/user_profile.dart';
import 'package:mlperfbench/ui/settings/about_screen.dart';
import 'package:mlperfbench/ui/settings/settings_screen.dart';

class AppDrawer extends StatelessWidget {
  const AppDrawer({super.key});

  @override
  Widget build(BuildContext context) {
    final header = buildHeader(context);
    final menuList = buildMenuList(context);
    return Drawer(
      backgroundColor: AppColors.drawerBackground,
      child: Theme(
        data: Theme.of(context).copyWith(
          textTheme: Theme.of(context).textTheme.apply(
                bodyColor: AppColors.drawerForeground,
                displayColor: AppColors.drawerForeground,
                decorationColor: AppColors.drawerForeground,
              ),
          listTileTheme: const ListTileThemeData(
            iconColor: AppColors.drawerForeground,
          ),
          iconTheme: const IconThemeData(
            color: AppColors.drawerForeground,
          ),
        ),
        child: Container(
          color: AppColors.drawerBackground,
          child: SafeArea(
            child: Column(
              children: [header] + menuList,
            ),
          ),
        ),
      ),
    );
  }

  Widget buildHeader(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    final appTitle = Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: Text(
        l10n.menuHome,
        style: const TextStyle(fontSize: 14, fontWeight: FontWeight.bold),
      ),
    );
    if (FirebaseManager.enabled) {
      return DrawerHeader(
        child: ListView(
          children: [
            appTitle,
            const UserProfileSection(),
          ],
        ),
      );
    } else {
      return SizedBox(
        height: 80,
        child: DrawerHeader(child: appTitle),
      );
    }
  }

  List<Widget> buildMenuList(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    return [
      ListTile(
        leading: const Icon(Icons.access_time),
        title: Text(l10n.menuHistory),
        onTap: () {
          Navigator.pop(context);
          Navigator.push(
            context,
            MaterialPageRoute(
              builder: (context) => const HistoryListScreen(),
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
      const Spacer(),
      const Divider(),
      ListTile(
        title: Text(l10n.settingsPrivacyPolicy),
        onTap: () => launchUrl(Uri.parse(Url.privacyPolicy)),
      ),
      ListTile(
        title: Text(l10n.settingsEula),
        onTap: () => launchUrl(Uri.parse(Url.eula)),
      ),
    ];
  }
}
