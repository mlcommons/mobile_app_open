import 'dart:io';

import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/resources/cache_manager.dart';

void main() async {
  group('CacheManager', () {
    late CacheManager manager;
    const zipRemotePath =
        'https://mobile.mlcommons-storage.org/app-resources/datasets/v3_0/coco-test.zip';
    const txtRemotePath =
        'https://mobile.mlcommons-storage.org/app-resources/datasets/v3_0/imagenet_tiny-groundtruth.txt';
    final paths = [zipRemotePath, txtRemotePath];

    setUp(() async {
      manager = CacheManager('/tmp/resources');
      await manager.cache(paths, (val, str) {}, true, true);
    });

    test('get', () async {
      expect(manager.get('foo/bar'), isNull, reason: 'path should be null');
      final txtLocalPath = manager.get(txtRemotePath);
      expect(txtLocalPath, isNotNull,
          reason: 'path [$txtLocalPath] should not be null');
      final txtFile = File(txtLocalPath!);
      expect(await txtFile.exists(), isTrue,
          reason: 'file should exist at path [$txtLocalPath]');
      final zipLocalPath = manager.get(zipRemotePath);
      expect(zipLocalPath, isNotNull,
          reason: 'path [$zipLocalPath] should not be null');
      final zipDirectory = Directory(zipLocalPath!);
      expect(await zipDirectory.exists(), isTrue,
          reason: 'directory should exist at path [$zipLocalPath]');
    });

    test('getArchive', () {
      expect(manager.getArchive('/foo/bar'), isNull,
          reason: 'returned path should be null');
      expect(manager.getArchive(txtRemotePath), isNull,
          reason: 'returned path should be null');
      expect(manager.getArchive(zipRemotePath), isNotNull,
          reason: 'returned path should not be null');
    });

    test('isResourceAnArchive', () {
      expect(manager.isResourceAnArchive(txtRemotePath), isFalse,
          reason: 'resource should not be an archive');
      expect(manager.isResourceAnArchive(zipRemotePath), isTrue,
          reason: 'resource should be an archive');
    });

    test('deleteArchives', () async {
      await manager.deleteArchives(paths);
      final zipLocalPath = manager.getArchive(zipRemotePath);
      expect(zipLocalPath, isNotNull);
      expect(await File(zipLocalPath!).exists(), isFalse,
          reason: 'file should not exist at path [$zipLocalPath]');
      expect(manager.getArchive(txtRemotePath), isNull);
    });

    test('deleteFiles', () async {
      await manager.deleteFiles(paths);
      final txtLocalPath = manager.get(txtRemotePath);
      expect(txtLocalPath, isNotNull);
      expect(await File(txtLocalPath!).exists(), isFalse,
          reason: 'file should not exist at path [$txtLocalPath]');
    });

    test('deleteLoadedResources', () async {
      await manager.deleteLoadedResources(paths);

      final zipLocalFilePath = manager.getArchive(zipRemotePath);
      expect(zipLocalFilePath, isNotNull);
      expect(await File(zipLocalFilePath!).exists(), isFalse,
          reason: 'file should not exist at path [$zipLocalFilePath]');

      final zipLocalDirectoryPath = manager.get(zipRemotePath);
      expect(zipLocalDirectoryPath, isNotNull,
          reason: 'path [$zipLocalDirectoryPath] should not be null');
      final zipDirectory = Directory(zipLocalDirectoryPath!);
      expect(await zipDirectory.exists(), isFalse,
          reason: 'directory should exist at path [$zipLocalDirectoryPath]');

      final txtLocalPath = manager.get(txtRemotePath);
      expect(txtLocalPath, isNotNull);
      expect(await File(txtLocalPath!).exists(), isFalse,
          reason: 'file should not exist at path [$txtLocalPath]');
    });
  });
}
