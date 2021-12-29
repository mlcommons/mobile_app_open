import 'dart:io';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart' hide Icons;
import 'package:flutter/services.dart';

import 'package:provider/provider.dart';

import 'package:mlcommons_ios_app/benchmark/benchmark.dart';
import 'package:mlcommons_ios_app/localizations/app_localizations.dart';
import 'package:mlcommons_ios_app/store.dart';
import 'package:mlcommons_ios_app/ui/main_screen.dart';

// TODO sharing screen temporarily disabled
// import 'package:mlcommons_ios_app/ui/share_screen.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await SystemChrome.setPreferredOrientations(
      [DeviceOrientation.portraitUp, DeviceOrientation.portraitDown]);

  try {
    await launchUi();
  } catch (e, s) {
    print('Exception: $e');
    print('Exception stack: $s');
    runApp(ExceptionWidget(e, s));
  }
}

Future<void> launchUi() async {
  final store = await Store.create();
  final benchmarkState = await BenchmarkState.create(store);

  if (const bool.fromEnvironment('autostart', defaultValue: false)) {
    assert(const bool.hasEnvironment('resultsStringMark'));
    assert(const bool.hasEnvironment('terminalStringMark'));
    await store.deletePreviousResult();
    await benchmarkState.resourceManager.resultManager.delete();
    await benchmarkState.reset();
    benchmarkState.addListener(() => autostartHandler(benchmarkState, store));
  }

  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider.value(value: benchmarkState),
        ChangeNotifierProvider.value(value: store)
      ],
      child: MyApp(),
    ),
  );
}

void autostartHandler(BenchmarkState state, Store store) async {
  if (state.state == BenchmarkStateEnum.waiting) {
    store.submissionMode =
        const bool.fromEnvironment('submission', defaultValue: false);
    store.offlineMode =
        const bool.fromEnvironment('offline', defaultValue: false);
    state.runBenchmarks();
    return;
  }
  if (state.state == BenchmarkStateEnum.done) {
    print(const String.fromEnvironment('resultsStringMark'));
    print(await state.resourceManager.resultManager.read());
    print(const String.fromEnvironment('terminalStringMark'));
    exit(0);
  }
}

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    // TODO sharing screen temporarily disabled
    // final store = context.watch<Store>();

    return MaterialApp(
      title: 'MLPerf Mobile',
      localizationsDelegates: [AppLocalizations.delegate],
      supportedLocales: [const Locale('en', '')],
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
        primarySwatch: Colors.blue,
        // This makes the visual density adapt to the platform that you run
        // the app on. For desktop platforms, the controls will be smaller and
        // closer together (more dense) than on mobile platforms.
        visualDensity: VisualDensity.adaptivePlatformDensity,
        // This sets default color for active switcher
        toggleableActiveColor: Colors.green,
        scaffoldBackgroundColor: Colors.white,

        // This theme of application app bar
        appBarTheme: AppBarTheme(
          titleTextStyle: TextStyle(color: Colors.black, fontSize: 20),
          elevation: 0,
          backgroundColor: Colors.white,
          iconTheme: IconThemeData(color: Color(0xFF135384)),
        ),
      ),
      // TODO sharing screen temporarily disabled
      // home: store.isShareOptionChosen() ? MyHomePage() : ShareScreen(),
      home: MyHomePage(),
    );
  }
}

class ExceptionWidget extends StatelessWidget {
  final Object error;
  final StackTrace stackTrace;
  ExceptionWidget(this.error, this.stackTrace);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        title: 'MLPerf Mobile',
        home: Material(
          type: MaterialType.transparency,
          child: SafeArea(
              child: Text(
            'Error: $error\n\nStackTrace: $stackTrace',
            style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.normal,
                color: Colors.white),
          )),
        ));
  }
}
