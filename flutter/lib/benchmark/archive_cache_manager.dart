import 'dart:io';

import 'package:archive/archive_io.dart';

import 'cache_manager.dart';

class ArchiveCacheManager {
  static const _zipPattern = '.zip';

  final CacheManager cacheManager;

  ArchiveCacheManager(this.cacheManager);


  Future<bool> hasCached(String url) async {
    var cachedPath = cacheManager.getCachePath(url);
    return Directory(getArchiveFolder(cachedPath)).exists();
  }

  String getArchiveFolder(String cachedPath) {
    return cachedPath.substring(0, cachedPath.length - _zipPattern.length);
  }

  // If resource is cached, returns path to it
  // If resource doesn't exists and downloadMissing=true,
  //    downloads and returns path
  // If resource doesn't exists and downloadMissing=false,
  //    returns empty string
  Future<String> get(String url, bool downloadMissing) async {
    var cachePath = getArchiveFolder(cacheManager.getCachePath(url));
    if (await Directory(cachePath).exists()) {
      return cachePath;
    }
    if (!downloadMissing) {
      return '';
    }
    var file = await cacheManager.downloadFile(url);
    final unZippedDirectory = await _unzipFile(file);
    await file.delete();
    return unZippedDirectory.path;
  }

  Future<Directory> _unzipFile(File zippedFile) async {
    final result = Directory(getArchiveFolder(zippedFile.path));
    await result.create(recursive: true);

    try {
      final archive = ZipDecoder().decodeBytes(await zippedFile.readAsBytes());

      for (final archiveFile in archive) {
        final filePath = '${result.path}/${archiveFile.name}';
        final file = await File(filePath).create(recursive: true);

        final data = archiveFile.content as List<int>;
        await file.writeAsBytes(data);
      }

      return result;
    } catch (e) {
      await result.delete(recursive: true);
      throw 'Could not unzip file ${zippedFile.path}: $e';
    }
  }
}
