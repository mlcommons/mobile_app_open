import 'dart:convert';
import 'dart:io';

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/data/environment/environment_info.dart';
import 'package:mlperfbench/data/extended_result.dart';
import 'package:mlperfbench/data/result_filter.dart';

void main() {
  group('ResultFilter', () {
    const file = 'unit_test/data/extended_result_unittest.json';
    final jsonString = File(file).readAsStringSync();
    final data = jsonDecode(jsonString) as Map<String, dynamic>;
    final result = ExtendedResult.fromJson(data);

    test('creationDate matched', () {
      final filter = ResultFilter();
      filter.fromCreationDate = DateTime.parse('2023-04-06T00:00:00Z');
      filter.toCreationDate = DateTime.parse('2023-04-11T00:00:00Z');
      expect(filter.match(result), isTrue);
    });

    test('creationDate not matched', () {
      final filter = ResultFilter();
      filter.fromCreationDate = DateTime.parse('2023-04-01T00:00:00Z');
      filter.toCreationDate = DateTime.parse('2022-04-02T00:00:00Z');
      expect(filter.match(result), isFalse);
    });

    test('platform matched', () {
      final filter = ResultFilter()..platform = EnvPlatform.ios.name;
      expect(filter.match(result), isTrue);
    });

    test('platform not matched', () {
      final filter = ResultFilter()..platform = EnvPlatform.android.name;
      expect(filter.match(result), isFalse);
    });

    test('deviceModel matched', () {
      final filter = ResultFilter()..deviceModel = 'iPad Pro 11';
      expect(filter.match(result), isTrue);
    });

    test('deviceModel not matched', () {
      final filter = ResultFilter()..deviceModel = 'iPhone 14';
      expect(filter.match(result), isFalse);
    });

    test('backend matched', () {
      final filter = ResultFilter()..backend = 'libtflitebackend';
      expect(filter.match(result), isTrue);
    });

    test('backend not matched', () {
      final filter = ResultFilter()..backend = 'libtflitepixelbackend';
      expect(filter.match(result), isFalse);
    });

    test('manufacturer matched', () {
      final filter = ResultFilter()..manufacturer = 'Apple';
      expect(filter.match(result), isTrue);
    });

    test('manufacturer not matched', () {
      final filter = ResultFilter()..manufacturer = 'Google';
      expect(filter.match(result), isFalse);
    });

    test('soc matched', () {
      final filter = ResultFilter()..soc = 'A12Z Bionic';
      expect(filter.match(result), isTrue);
    });

    test('soc not matched', () {
      final filter = ResultFilter()..soc = 'sdm855';
      expect(filter.match(result), isFalse);
    });

    test('multiple filters matched', () {
      final filter = ResultFilter()
        ..platform = EnvPlatform.ios.name
        ..deviceModel = 'iPad Pro 11'
        ..backend = 'libtflitebackend';
      expect(filter.match(result), isTrue);
    });

    test('multiple filters matched', () {
      final filter = ResultFilter()
        ..platform = EnvPlatform.ios.name
        ..deviceModel = 'iPad Pro 11'
        ..backend = 'libtflitepixelbackend';
      expect(filter.match(result), isFalse);
    });

    test('anyFilterActive is true', () {
      final filter = ResultFilter();
      filter.benchmarkId = 'image_classification';
      expect(filter.anyFilterActive, isTrue);
    });

    test('anyFilterActive is false', () {
      final filter = ResultFilter();
      expect(filter.anyFilterActive, isFalse);
    });

    test('empty result list', () {
      final filter = ResultFilter();
      final emptyResult = ExtendedResult.fromJson(data);
      emptyResult.results.clear();
      expect(filter.match(emptyResult), isFalse);
    });

    test('backend filter matches any result in a mixed-backend file', () {
      final mixedData = jsonDecode(jsonString) as Map<String, dynamic>;
      final results = mixedData['results'] as List<dynamic>;
      final lastBackendInfo =
          (results.last as Map<String, dynamic>)['backend_info']
              as Map<String, dynamic>;
      lastBackendInfo['filename'] = 'libcoremlbackend';
      final mixedResult = ExtendedResult.fromJson(mixedData);

      final coremlFilter = ResultFilter()..backend = 'libcoremlbackend';
      expect(coremlFilter.match(mixedResult), isTrue);

      final tfliteFilter = ResultFilter()..backend = 'libtflitebackend';
      expect(tfliteFilter.match(mixedResult), isTrue);

      final otherFilter = ResultFilter()..backend = 'libqtibackend';
      expect(otherFilter.match(mixedResult), isFalse);
    });
  });
}
