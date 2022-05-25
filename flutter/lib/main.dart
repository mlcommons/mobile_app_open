import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:mlperfbench_common/firebase/manager.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/backend/unsupported_device_exception.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/main_screen.dart';
import 'package:mlperfbench/ui/unsupported_device_screen.dart';
import 'device_info.dart';

// TODO sharing screen temporarily disabled
// import 'package:mlperfbench/ui/share_screen.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await SystemChrome.setPreferredOrientations(
      [DeviceOrientation.portraitUp, DeviceOrientation.portraitDown]);

  try {
    await launchUi();
  } on UnsupportedDeviceException catch (e) {
    runApp(MyApp(UnsupportedDeviceScreen(
      backendError: e.backendError,
    )));
  } catch (e, s) {
    print('Exception: $e');
    print('Exception stack: $s');
    runApp(ExceptionWidget(e, s));
  }
}

Future<void> launchUi() async {
  await DeviceInfo.staticInit();
  await BuildInfoHelper.staticInit();
  await FirebaseManager.staticInit();
  final store = await Store.create();
  final benchmarkState =
      await BenchmarkState.create(store, FirebaseManager.instance);

  if (const bool.fromEnvironment('autostart', defaultValue: false)) {
    assert(const bool.hasEnvironment('resultsStringMark'));
    assert(const bool.hasEnvironment('terminalStringMark'));
    await store.deletePreviousExtendedResult();
    await benchmarkState.resourceManager.resultManager.deleteLastResult();
    await benchmarkState.reset();
    benchmarkState.addListener(() => autostartHandler(benchmarkState, store));
  }

  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider.value(value: benchmarkState),
        ChangeNotifierProvider.value(value: store)
      ],
      child: MyApp(MyHomePage()),
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
    print(await state.resourceManager.resultManager.readLastResult());
    print(const String.fromEnvironment('terminalStringMark'));
    exit(0);
  }
}

class MyApp extends StatelessWidget {
  final Widget home;

  MyApp(this.home);

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
        primarySwatch: AppColors.primary,
        // This makes the visual density adapt to the platform that you run
        // the app on. For desktop platforms, the controls will be smaller and
        // closer together (more dense) than on mobile platforms.
        visualDensity: VisualDensity.adaptivePlatformDensity,
        // This sets default color for active switcher
        toggleableActiveColor: AppColors.secondary,
        scaffoldBackgroundColor: AppColors.lightBackground,

        // This theme of application app bar
        appBarTheme: AppBarTheme(
          titleTextStyle: TextStyle(color: AppColors.darkText, fontSize: 20),
          elevation: 0,
          backgroundColor: AppColors.lightAppBarBackground,
          iconTheme: IconThemeData(color: AppColors.darkAppBarIconTheme),
        ),
      ),
      // TODO sharing screen temporarily disabled
      // home: store.isShareOptionChosen() ? MyHomePage() : ShareScreen(),
      home: home,
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
                color: AppColors.lightText),
          )),
        ));
  }
}
