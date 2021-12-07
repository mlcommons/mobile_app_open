import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:path_provider/path_provider.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'dart:io';
import 'dart:convert';

import 'package:mlcommons_ios_app/main.dart' as app;
import 'package:mlcommons_ios_app/ui/main_screen.dart';
import 'package:mlcommons_ios_app/ui/result_screen.dart';
import 'package:mlcommons_ios_app/benchmark/resource_manager.dart'
    as resource_manager;

void main() {
  final splashPauseSeconds = 4;
  final runTimeLimitMinutes = 20;
  final downloadTimeLimitMinutes = 10;
  final secondsInMinute = 60;

  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  SharedPreferences.setMockInitialValues({'test mode': true});

  group('Testing App Performance Tests', () {
    final binding = IntegrationTestWidgetsFlutterBinding.ensureInitialized()
        as IntegrationTestWidgetsFlutterBinding;

    binding.framePolicy = LiveTestWidgetsFlutterBindingFramePolicy.fullyLive;

    testWidgets('Favorites operations test', (WidgetTester tester) async {
      Future<bool> waitFor(int timeLimitMinutes, Key key) async {
        var element = false;

        for (var counter = 0;
            counter < timeLimitMinutes * secondsInMinute;
            counter++) {
          await tester.pumpAndSettle(Duration(seconds: 1));
          final searchResult = find.byKey(key);

          if (tester.any(searchResult)) {
            element = true;
            break;
          }
        }

        return element;
      }

      await app.main();
      await tester.pumpAndSettle(Duration(seconds: splashPauseSeconds));

      var goButtonIsPresented =
          await waitFor(downloadTimeLimitMinutes, Key(MainKeys.goButton));

      expect(goButtonIsPresented, true,
          reason: 'Problems with downloading of datasets or models');
      final goButton = find.byKey(Key(MainKeys.goButton));
      await tester.tap(goButton);

      var scrollButtonIsPresented = await waitFor(
          runTimeLimitMinutes, Key(ResultKeys.scrollResultsButton));

      expect(scrollButtonIsPresented, true,
          reason: 'Test results were not found');

      final applicationDirectory =
          await resource_manager.ResourceManager.getApplicationDirectory();
      final jsonResultPath = '$applicationDirectory/result.json';
      final file = File(jsonResultPath);

      expect(await file.exists(), true,
          reason:
              'Result.json does not exist: file $applicationDirectory/result.json is not found');

      final jsonResultContent = await file.readAsString();
      final results = jsonDecode(jsonResultContent);
      final length = results.length;

      expect(length, 5, reason: 'results count should be 5, but it is $length');

      try {
        for (final resultContent in results) {
          final result = resultContent as Map<String, dynamic>;
          final id = result['benchmark_id'] as String;
          final accuracy = result['accuracy'] as String?;
          final score = result['score'] as double?;
          expect(accuracy != 'N/A', true,
              reason:
                  'accuracy should be N/A in benchmark $id, but is "$accuracy"');
          expect(score != null, true,
              reason: 'score should not be null in benchmark $id');
        }
      } catch (_) {
        stderr.writeln('Error in result.json format');
      }
    });
  });
}
