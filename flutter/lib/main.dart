import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/backend/unsupported_device_exception.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/root/app.dart';
import 'package:mlperfbench/ui/root/exception_screen.dart';
import 'package:mlperfbench/ui/root/main_screen.dart';
import 'package:mlperfbench/ui/root/unsupported_device_screen.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await SystemChrome.setPreferredOrientations(
      [DeviceOrientation.portraitUp, DeviceOrientation.portraitDown]);

  try {
    await launchUi();
  } on UnsupportedDeviceException catch (e) {
    runApp(MyApp(
      home: UnsupportedDeviceScreen(
        backendError: e.backendError,
      ),
    ));
  } catch (e, s) {
    print('Exception: $e');
    print('Exception stack: $s');
    runApp(ExceptionWidget(e, s));
  }
}

Future<void> launchUi() async {
  await DeviceInfo.staticInit();
  await BuildInfoHelper.staticInit();
  final store = await Store.create();
  final benchmarkState = await BenchmarkState.create(store);

  if (const bool.fromEnvironment('autostart', defaultValue: false)) {
    assert(const bool.hasEnvironment('resultsStringMark'));
    assert(const bool.hasEnvironment('terminalStringMark'));
    store.previousExtendedResult = '';
    benchmarkState.restoreLastResult();
    benchmarkState.addListener(() => autostartHandler(benchmarkState, store));
  }

  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider.value(value: benchmarkState),
        ChangeNotifierProvider.value(value: store)
      ],
      child: const MyApp(home: MyHomePage()),
    ),
  );
}

void autostartHandler(BenchmarkState state, Store store) async {
  if (state.state == BenchmarkStateEnum.waiting) {
    store.selectedBenchmarkRunMode =
        const bool.fromEnvironment('submission', defaultValue: false)
            ? BenchmarkRunModeEnum.submissionRun
            : BenchmarkRunModeEnum.performanceOnly;
    store.offlineMode =
        const bool.fromEnvironment('offline', defaultValue: false);
    await state.runBenchmarks();
    return;
  }
  if (state.state == BenchmarkStateEnum.done) {
    print(const String.fromEnvironment('resultsStringMark'));
    final result = state.resourceManager.resultManager.getLastResult();
    print(jsonToStringIndented(result));
    print(const String.fromEnvironment('terminalStringMark'));
    exit(0);
  }
}
