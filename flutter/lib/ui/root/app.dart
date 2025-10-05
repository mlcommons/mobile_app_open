import 'package:flutter/material.dart';

import 'package:bot_toast/bot_toast.dart';
import 'package:upgrader/upgrader.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/app_styles.dart';

class MyApp extends StatelessWidget {
  final Widget home;

  const MyApp({super.key, required this.home});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'MLPerf Mobile',
      localizationsDelegates: AppLocalizations.localizationsDelegates,
      supportedLocales: AppLocalizations.supportedLocales,
      theme: Theme.of(context).copyWith(
        // TODO: https://docs.flutter.dev/release/breaking-changes/material-3-migration
        // ignore: deprecated_member_use
        useMaterial3: false,
        colorScheme: ColorScheme.fromSwatch().copyWith(
          primary: AppColors.primary,
          secondary: AppColors.secondary,
        ),
        appBarTheme: const AppBarTheme(
          centerTitle: true,
          titleTextStyle: TextStyle(color: AppColors.lightText, fontSize: 20),
          elevation: 0,
          backgroundColor: AppColors.primaryAppBarBackground,
          iconTheme: IconThemeData(color: AppColors.appBarIcon),
        ),
        popupMenuTheme: PopupMenuThemeData(
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
          ),
        ),
      ),
      builder: BotToastInit(),
      navigatorObservers: [BotToastNavigatorObserver()],
      home: UpgradeAlert(child: home),
    );
  }
}
