import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:mlperfbench_common/firebase/manager.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/backend/bridge/isolate.dart';
import 'package:mlperfbench/backend/list.dart';
import 'package:mlperfbench/backend/unsupported_device_exception.dart';
import 'package:mlperfbench/board_decoder.dart';
import 'package:mlperfbench/build_info.dart';
import 'package:mlperfbench/resources/config_manager.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/result_manager.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/state/app_state.dart';
import 'package:mlperfbench/state/last_result_manager.dart';
import 'package:mlperfbench/state/task_list_manager.dart';
import 'package:mlperfbench/state/task_runner.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/root/main_screen/main_screen.dart';
import 'package:mlperfbench/ui/root/unsupported_device_screen.dart';
import 'device_info.dart';
import 'ui/root/app.dart';
import 'ui/root/exception_screen.dart';

// TODO sharing screen temporarily disabled
// import 'package:mlperfbench/ui/share_screen.dart';

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
  await FirebaseManager.staticInit();
  final store = await Store.create();
  final bridgeIsolate = await BridgeIsolate.create();
  final backendInfo = BackendInfoHelper().findMatching();

  final resourceDir = await ResourceManager.getApplicationDirectory();
  final resultManager = await ResultManager.create(resultDir: resourceDir);
  final resourceManager = await ResourceManager.create(
    store: store,
    resourceDir: resourceDir,
    resultManager: resultManager,
  );
  final configManager = await ConfigManager.create(
    applicationDirectory: resourceManager.resourceDir,
    resourceManager: resourceManager,
  );
  final taskRunner = TaskRunner(
    store: store,
    resourceManager: resourceManager,
    backendBridge: bridgeIsolate,
    backendInfo: backendInfo,
  );
  final lastResultManager = LastResultManager(store);
  lastResultManager.tryRestore();
  final taskListManager = TaskListManager(
    backendSettings: backendInfo.settings,
  );

  final appStateHelper = AppStateHelper(
    store: store,
    taskListManager: taskListManager,
    resourceManager: resourceManager,
    configManager: configManager,
    taskRunner: taskRunner,
    lastResultManager: lastResultManager,
  );
  final benchmarkState = await AppState.create(
    appStateHelper: appStateHelper,
  );

  if (const bool.fromEnvironment('autostart', defaultValue: false)) {
    assert(const bool.hasEnvironment('resultsStringMark'));
    assert(const bool.hasEnvironment('terminalStringMark'));
    lastResultManager.value = null;
    benchmarkState.addListener(
        () => autostartHandler(lastResultManager, benchmarkState, store));
  }

  final BoardDecoder boardDecoder = BoardDecoder();
  await boardDecoder.init();

  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider.value(value: benchmarkState),
        ChangeNotifierProvider.value(value: store),
        Provider.value(value: FirebaseManager.instance),
        Provider.value(value: boardDecoder),
        Provider.value(value: configManager),
        Provider.value(value: resourceManager),
        Provider.value(value: resultManager),
        Provider.value(value: lastResultManager),
        Provider.value(value: taskListManager),
      ],
      child: const MyApp(home: MyHomePage()),
    ),
  );
}

void autostartHandler(
    LastResultManager lastResultManager, AppState state, Store store) async {
  if (state.state != AppStateEnum.ready) {
    return;
  }
  if (lastResultManager.value == null) {
    store.submissionMode =
        const bool.fromEnvironment('submission', defaultValue: false);
    store.offlineMode =
        const bool.fromEnvironment('offline', defaultValue: false);
    state.startBenchmark();
    return;
  } else {
    print(const String.fromEnvironment('resultsStringMark'));
    final resourceDir = await ResourceManager.getApplicationDirectory();
    final resultManager = await ResultManager.create(resultDir: resourceDir);
    final result = resultManager.getLastResult();
    print(jsonToStringIndented(result));
    print(const String.fromEnvironment('terminalStringMark'));
    exit(0);
  }
}
