// The analyzer does not treat this non-standard test dir as a test context.
// ignore_for_file: invalid_use_of_visible_for_testing_member

import 'dart:io';

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/benchmark/info.dart';
import 'package:mlperfbench/localizations/app_localizations_en.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/ui/icons.dart';

/// Ids of every `task {}` block in the shipped config.
///
/// The config is the source of truth the app loads at runtime, so anything
/// keyed by task id has to cover all of it. Read rather than hardcoded, so a
/// task added to the pbtxt fails here instead of at the user's fingertip.
List<String> taskIdsFromConfig() {
  final file = File('assets/tasks.pbtxt');
  if (!file.existsSync()) {
    throw StateError('run this from the flutter/ directory: ${file.path}');
  }
  final idPattern = RegExp(r'^\s*id:\s*"([^"]+)"', multiLine: true);
  return file
      .readAsStringSync()
      .split(RegExp(r'^task \{', multiLine: true))
      .skip(1) // the leading chunk holds the task_set blocks
      .map((chunk) => idPattern.firstMatch(chunk)?.group(1))
      .whereType<String>()
      .toList();
}

void main() {
  group('every shipped task is covered', () {
    final taskIds = taskIdsFromConfig();
    final l10n = AppLocalizationsEn();

    test('the config parses into a plausible task list', () {
      expect(taskIds, contains('image_classification_v2'));
      expect(taskIds, contains('llm-8b-instruct'));
      expect(taskIds.length, greaterThanOrEqualTo(13));
    });

    test('BenchmarkId.allIds lists every task', () {
      // allIds drives the sort in BenchmarkStore via indexOf, which returns
      // -1 for anything missing — so an omitted task jumps to the front.
      final missing = taskIds
          .where((id) => !BenchmarkId.allIds.contains(id))
          .toList();
      expect(missing, isEmpty, reason: 'missing from BenchmarkId.allIds');
    });

    test('getLocalizedInfo handles every task', () {
      // The default branch throws, and showBenchInfoBottomSheet is its only
      // caller, so a gap here is a crash on the info button.
      final failed = <String>[];
      for (final id in taskIds) {
        try {
          BenchmarkInfo(pb.TaskConfig(id: id)).getLocalizedInfo(l10n);
        } catch (_) {
          failed.add(id);
        }
      }
      expect(failed, isEmpty, reason: 'getLocalizedInfo throws for these');
    });

    test('every task has its own icon', () {
      final missing = taskIds
          .where((id) => !BenchmarkIcons.darkSet.containsKey(id))
          .toList();
      expect(missing, isEmpty, reason: 'fall back to the generic logo');
    });
  });
}
