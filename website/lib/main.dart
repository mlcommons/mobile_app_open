import 'package:flutter/material.dart';

import 'package:mlperfbench_common/firebase/manager.dart';

import 'package:website/app_state.dart';
import 'package:website/route_generator.dart';
import 'app_constants.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  try {
    await staticInit();
    // This app can't function without access to Firebase
    // so we can assume that FirebaseManager.instance is never null
    AppState.instance = AppState(FirebaseManager.instance!);
    // AppState.instance.fetchResults();
    runApp(const MyApp());
  } catch (e, s) {
    print('Exception: $e');
    print('Exception stack: $s');
    runApp(ExceptionWidget(error: e, stackTrace: s));
  }
}

Future<void> staticInit() async {
  await FirebaseManager.staticInit();
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      initialRoute: '/',
      onGenerateRoute: RouteGenerator.generateRoute,
    );
  }
}

class ExceptionWidget extends StatelessWidget {
  final Object error;
  final StackTrace stackTrace;

  const ExceptionWidget(
      {Key? key, required this.error, required this.stackTrace})
      : super(key: key);

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
