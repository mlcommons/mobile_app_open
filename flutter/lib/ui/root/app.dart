import 'package:flutter/material.dart';

import 'package:bot_toast/bot_toast.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class MyApp extends StatelessWidget {
  final Widget home;

  const MyApp({Key? key, required this.home}) : super(key: key);

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'MLPerf Mobile',
      localizationsDelegates: const [AppLocalizations.delegate],
      supportedLocales: const [Locale('en', '')],
      theme: Theme.of(context).copyWith(
        appBarTheme: const AppBarTheme(
          centerTitle: true,
          titleTextStyle: TextStyle(color: AppColors.lightText, fontSize: 20),
          elevation: 0,
          backgroundColor: AppColors.appBarBackground,
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
      home: home,
    );
  }
}
