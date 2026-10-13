import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/resources/resource.dart';
import 'package:mlperfbench/resources/utils.dart';

Resource _resource(String path) =>
    Resource(type: ResourceTypeEnum.model, path: path, md5Checksum: '');

void main() {
  group('isInternetResource', () {
    test('accepts http and https', () {
      expect(isInternetResource('http://example.com/model.tflite'), isTrue);
      expect(isInternetResource('https://example.com/model.tflite'), isTrue);
    });
    test('rejects other schemes and local paths', () {
      expect(isInternetResource('ftp://example.com/model.tflite'), isFalse);
      expect(isInternetResource('asset://local/model.tflite'), isFalse);
      expect(isInternetResource('/data/local/model.tflite'), isFalse);
      expect(isInternetResource(''), isFalse);
    });
  });

  group('asset uri', () {
    test('isAsset', () {
      expect(isAsset('asset://local/model.tflite'), isTrue);
      expect(isAsset('https://example.com/model.tflite'), isFalse);
      expect(isAsset('local/model.tflite'), isFalse);
    });
    test('stripAssetPrefix', () {
      expect(
        stripAssetPrefix('asset://local/model.tflite'),
        'local/model.tflite',
      );
      expect(stripAssetPrefix('asset://'), '');
    });
  });

  group('filterInternetResources', () {
    test('keeps only internet resources, in order', () {
      final resources = [
        _resource('https://example.com/a.tflite'),
        _resource('asset://local/b.tflite'),
        _resource('/data/c.tflite'),
        _resource('http://example.com/d.tflite'),
      ];
      expect(filterInternetResources(resources), [
        'https://example.com/a.tflite',
        'http://example.com/d.tflite',
      ]);
    });
    test('returns empty list when there is nothing to download', () {
      expect(filterInternetResources([]), isEmpty);
      expect(filterInternetResources([_resource('asset://local/b')]), isEmpty);
    });
  });

  group('jsonToStringIndented', () {
    test('indents with two spaces', () {
      final json = {
        'name': 'mlperf',
        'values': [1, 2],
      };
      expect(
        jsonToStringIndented(json),
        '{\n'
        '  "name": "mlperf",\n'
        '  "values": [\n'
        '    1,\n'
        '    2\n'
        '  ]\n'
        '}',
      );
    });
  });

  group('lerpRange', () {
    test('returns start and end at the bounds of the value range', () {
      expect(lerpRange(0, 100, 10, 20, 10), 0);
      expect(lerpRange(0, 100, 10, 20, 20), 100);
    });
    test('interpolates inside the value range', () {
      expect(lerpRange(0, 100, 10, 20, 15), 50);
      expect(lerpRange(200, 100, 0, 4, 1), 175);
    });
    test('extrapolates outside the value range', () {
      expect(lerpRange(0, 100, 10, 20, 25), 150);
    });
    test('returns null when both ends are null', () {
      expect(lerpRange(null, null, 0, 1, 0.5), isNull);
    });
  });
}
