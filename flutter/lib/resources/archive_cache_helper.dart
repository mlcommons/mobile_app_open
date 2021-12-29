import 'dart:io';

import 'package:archive/archive_io.dart';

import 'file_cache_helper.dart';

class ArchiveCacheHelper {
  static const extension = '.zip';
  final FileCacheHelper fileCacheHelper;

  ArchiveCacheHelper(this.fileCacheHelper);

  String _getArchiveFolder(String cachedPath) {
    return cachedPath.substring(0, cachedPath.length - extension.length);
  }

  // If archive is cached, returns path to it
  // If archive is not cached and downloadMissing=true,
  //    downloads it and returns path to folder
  // If archive is not cached and downloadMissing=false,
  //    returns empty string
  Future<String> get(String url, bool downloadMissing) async {
    var cachePath = _getArchiveFolder(fileCacheHelper.getCachePath(url));
    if (await Directory(cachePath).exists()) {
      return cachePath;
    }
    if (!downloadMissing) {
      return '';
    }
    var file = await fileCacheHelper.get(url, true);
    await _unzipFile(file);
    await File(file).delete();
    return cachePath;
  }

  Future<Directory> _unzipFile(String archivePath) async {
    final result = Directory(_getArchiveFolder(archivePath));
    await result.create(recursive: true);

    try {
      final archive =
          ZipDecoder().decodeBytes(await File(archivePath).readAsBytes());

      for (final archiveFile in archive) {
        final filePath = '${result.path}/${archiveFile.name}';
        final file = await File(filePath).create(recursive: true);

        final data = archiveFile.content as List<int>;
        await file.writeAsBytes(data);
      }

      return result;
    } catch (e) {
      await result.delete(recursive: true);
      throw 'Could not unzip file $archivePath: $e';
    }
  }
}
