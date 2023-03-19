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
    // TODO sharing screen temporarily disabled
    // final store = context.watch<Store>();

    return MaterialApp(
      title: 'MLPerf Mobile',
      localizationsDelegates: const [AppLocalizations.delegate],
      supportedLocales: const [Locale('en', '')],
      theme: ThemeData(
        // This is the theme of your application.
        //
        // Try running your application with "flutter run". You'll see the
        // application has a blue toolbar. Then, without quitting the app, try
        // changing the primarySwatch below to Colors.green and then invoke
        // "hot reload" (press "r" in the console where you ran "flutter run",
        // or simply save your changes to "hot reload" in a Flutter IDE).
        // Notice that the counter didn't reset back to zero; the application
        // is not restarted.
        primarySwatch: AppColors.primary,
        // This makes the visual density adapt to the platform that you run
        // the app on. For desktop platforms, the controls will be smaller and
        // closer together (more dense) than on mobile platforms.
        visualDensity: VisualDensity.adaptivePlatformDensity,
        // This sets default color for active switcher
        toggleableActiveColor: AppColors.secondary,
        scaffoldBackgroundColor: AppColors.lightBackground,

        // This theme of application app bar
        appBarTheme: const AppBarTheme(
          titleTextStyle: TextStyle(color: AppColors.lightText, fontSize: 20),
          elevation: 0,
          backgroundColor: AppColors.darkAppBarThemeBackground,
          iconTheme: IconThemeData(color: AppColors.lightAppBarIconTheme),
        ),
      ),
      builder: BotToastInit(),
      navigatorObservers: [BotToastNavigatorObserver()],
      // TODO sharing screen temporarily disabled
      // home: store.isShareOptionChosen() ? MyHomePage() : ShareScreen(),
      home: home,
    );
  }
}
