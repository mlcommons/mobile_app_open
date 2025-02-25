import 'dart:io';

import 'package:mlperfbench/resources/archive_cache_helper.dart';
import 'package:mlperfbench/resources/file_cache_helper.dart';

const _oldFilesAgeInDays = 30 * 9;

class CacheManager {
  final String loadedResourcesDir;
  late final FileCacheHelper fileCacheHelper;
  late final ArchiveCacheHelper archiveCacheHelper;
  Map<String, String> _resourcesMap = {};

  CacheManager(this.loadedResourcesDir) {
    fileCacheHelper = FileCacheHelper(loadedResourcesDir);
    archiveCacheHelper = ArchiveCacheHelper(fileCacheHelper);
  }

  String? get(String url) {
    return _resourcesMap[url];
  }

  String? getArchive(String url) {
    if (!isResourceAnArchive(url)) return null;
    final archiveDirPath = get(url);
    if (archiveDirPath == null) return null;
    final archiveFilePath = archiveDirPath + ArchiveCacheHelper.extension;
    return archiveFilePath;
  }

  Future<void> deleteLoadedResources(List<String> excludes,
      [int atLeastDaysOld = 0]) async {
    final directory = Directory(loadedResourcesDir);

    // can't use 'await for' here because we delete files,
    // and 'await for' on stream from directory.list() throws exceptions when file is missing
    for (final file in await directory.list(recursive: true).toList()) {
      // skip files in already deleted folders
      if (!await file.exists()) continue;
      final relativePath = file.path
          .replaceAll('\\', '/')
          .substring(loadedResourcesDir.length + 1);
      var keep = false;
      for (var resource in excludes) {
        // relativePath.startsWith(resource): if we want to preserve a folder resource
        // resource.startsWith(relativePath): if we want to preserve a file resource
        //   for example:
        //   we are checking folder 'github.com'
        //   resource is 'github.com/mlcommons/mobile_models/raw/main/v0_7/datasets/ade20k'
        if (relativePath.startsWith(resource) ||
            resource.startsWith(relativePath)) {
          keep = true;
          break;
        }
      }
      if (keep) continue;
      if (atLeastDaysOld > 0) {
        var stat = await file.stat();
        if (DateTime.now().difference(stat.modified).inDays < atLeastDaysOld) {
          continue;
        }
      }
      await file.delete(recursive: true);
    }
  }

  Future<void> deleteArchives(List<String> resources) async {
    for (final resource in resources) {
      final archivePath = getArchive(resource);
      if (archivePath == null) continue;
      final archiveFile = File(archivePath);
      if (await archiveFile.exists()) await archiveFile.delete();
    }
  }

  Future<void> deleteFiles(List<String> resources) async {
    for (final resource in resources) {
      final filePath = get(resource);
      if (filePath == null) continue;
      final file = File(filePath);
      if (await file.exists()) await file.delete();
      print('Deleted resource $resource stored at ${file.path}');
    }
  }

  Future<void> purgeOutdatedCache(int atLeastDaysOld) async {
    var currentResources = <String>[];
    for (var r in _resourcesMap.values) {
      currentResources.add(r.substring(loadedResourcesDir.length + 1));
    }
    return deleteLoadedResources(currentResources, atLeastDaysOld);
  }

  Future<void> cache({
    required List<String> urls,
    required void Function(double, String) onProgressUpdate,
    required bool purgeOldCache,
    required bool downloadMissing,
  }) async {
    final resourcesToDownload = <String>[];
    _resourcesMap = {};

    for (final resource in urls) {
      if (_resourcesMap.containsKey(resource)) continue;
      if (resourcesToDownload.contains(resource)) continue;

      String path;
      if (isResourceAnArchive(resource)) {
        path = await archiveCacheHelper.get(resource, false);
      } else {
        path = await fileCacheHelper.get(resource, false);
      }

      if (path != '') {
        _resourcesMap[resource] = path;
        continue;
      }

      resourcesToDownload.add(resource);

      continue;
    }
    if (downloadMissing) {
      await _download(resourcesToDownload, onProgressUpdate);
    }
    if (purgeOldCache) {
      await purgeOutdatedCache(_oldFilesAgeInDays);
    }
  }

  bool isResourceAnArchive(String resource) {
    return resource.endsWith(ArchiveCacheHelper.extension);
  }

  Future<void> _download(
    List<String> urls,
    void Function(double, String) onProgressUpdate,
  ) async {
    var progress = 0.0;
    for (var url in urls) {
      progress += 0.1 / urls.length;
      onProgressUpdate(progress, url);
      if (isResourceAnArchive(url)) {
        _resourcesMap[url] = await archiveCacheHelper.get(url, true);
      } else {
        _resourcesMap[url] = await fileCacheHelper.get(url, true);
      }
      progress += 0.9 / urls.length;
      onProgressUpdate(progress, url);
    }
  }
}
