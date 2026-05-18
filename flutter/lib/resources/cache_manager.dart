import 'dart:io';

import 'package:mlperfbench/resources/archive_cache_helper.dart';
import 'package:mlperfbench/resources/file_cache_helper.dart';

const _oldFilesAgeInDays = 30 * 9;

class CacheManager {
  final String loadedResourcesDir;
  late final FileCacheHelper fileCacheHelper;
  late final ArchiveCacheHelper archiveCacheHelper;
  Map<String, String> _resourcesMap = {};
  List<String> newDownloads = [];

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
    final Map<String, String> resourcesToDelete = Map.from(_resourcesMap);
    final deletionTime = DateTime.now();

    for (var key in excludes) {
      resourcesToDelete.remove(key);
    }

    for (var resource in resourcesToDelete.keys.toList()) {
      String resourcePath = resourcesToDelete[resource] ?? '';
      var stat = FileStat.statSync(resourcePath);
      if (stat.type == FileSystemEntityType.notFound ||
          (atLeastDaysOld > 0 &&
              deletionTime.difference(stat.modified).inDays < atLeastDaysOld)) {
        resourcesToDelete.remove(resource);
        continue;
      }
      try {
        switch (stat.type) {
          case FileSystemEntityType.file:
          case FileSystemEntityType.link:
            File(resourcePath).deleteSync(recursive: true);
            break;
          case FileSystemEntityType.directory:
            Directory(resourcePath).deleteSync(recursive: true);
            break;
          default:
            break;
        }
        _resourcesMap.remove(resource);
      } on FileSystemException catch (e) {
        if (e.osError?.errorCode == 2) {
          // TODO might need to be changed for Windows
          _resourcesMap.remove(resource);
        } else
          rethrow;
      }
    }
  }

  // NOTE this does not remove the files that were extracted from the archive, just the archive itself.
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
      _resourcesMap.remove(
          resource); // Update the resource map to reflect the deleted file
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
    bool overrideMap = false,
  }) async {
    final resourcesToDownload = <String>[];
    final resourcesMap = <String, String>{};
    newDownloads.clear();

    for (final resource in urls) {
      if (resourcesMap.containsKey(resource)) continue;
      if (resourcesToDownload.contains(resource)) continue;

      String path;
      if (isResourceAnArchive(resource)) {
        path = await archiveCacheHelper.get(resource, false);
      } else {
        path = await fileCacheHelper.get(resource, false);
      }

      if (path != '') {
        resourcesMap[resource] = path;
        continue;
      }

      resourcesToDownload.add(resource);

      continue;
    }
    if (downloadMissing) {
      await _download(resourcesToDownload, onProgressUpdate, resourcesMap);
    }
    if (overrideMap) {
      _resourcesMap = resourcesMap;
    } else {
      _resourcesMap.addAll(resourcesMap);
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
      Map<String, String> resourcesMap // Passed by reference
      ) async {
    newDownloads = urls;
    var progress = 0.0;
    for (var url in urls) {
      progress += 0.1 / urls.length;
      onProgressUpdate(progress, url);
      if (isResourceAnArchive(url)) {
        resourcesMap[url] = await archiveCacheHelper.get(url, true);
      } else {
        resourcesMap[url] = await fileCacheHelper.get(url, true);
      }
      progress += 0.9 / urls.length;
      onProgressUpdate(progress, url);
    }
  }
}
