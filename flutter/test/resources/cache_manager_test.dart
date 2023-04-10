import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:mlperfbench/resources/cache_manager.dart';

void main() async {
  group('CacheManager', () {
    late CacheManager manager;
    final paths = [
      'https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-zip-file.zip',
    ];
    setUp(() async {
      manager = CacheManager('/tmp/resources');
      await manager.cache(paths, (val) {}, true);
    });

    test('get', () async {
      expect(manager.get('non-existent'), isNull,
          reason: 'path should be null');
      final localPath = manager.get(paths[0]);
      expect(localPath, isNotNull, reason: 'path should not be null');
      final file = File(localPath!);
      expect(await file.exists(), isTrue, reason: 'file should exist');
    });

    test('getArchive', () {
      expect(manager.getArchive('/downloads/file'), isNull,
          reason: 'returned path should be null');
      expect(manager.getArchive(paths[0]), isNotNull,
          reason: 'returned path should not be null');
    });

    test('isResourceAnArchive', () {
      expect(manager.isResourceAnArchive('/downloads/file'), isFalse,
          reason: 'resource should not be an archive');
      expect(manager.isResourceAnArchive('/downloads/file.zip'), isTrue,
          reason: 'resource should be an archive');
    });

    test('deleteArchives', () async {
      await manager.deleteArchives(paths);
      expect(manager.getArchive(paths[0]), isNull);
    });
  });
}
