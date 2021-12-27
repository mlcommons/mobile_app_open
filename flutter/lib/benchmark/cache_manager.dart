
import 'dart:io';

import 'archive_cache_helper.dart';
import 'file_cache_helper_.dart';

class CacheManager {
  final String loadedResourcesDir;
  final FileCacheHelper fileCacheHelper;
  final ArchiveCacheHelper archiveCacheHelper;
  Map<String, String> _resourcesMap = {};

  CacheManager(this.loadedResourcesDir, this.fileCacheHelper, this.archiveCacheHelper);

  String? get(String url) {
    return _resourcesMap[url];
  }
  
  Future<void> deleteLoadedResources(List<String> nonRemovableResources,
      [int atLeastDaysOld = 0]) async {
    final directory = Directory(loadedResourcesDir);

    // can't use await for here because we delete files,
    // and await for on stream from .list() throws exceptions when file is missing
    for (final file in await directory.list(recursive: true).toList()) {
      // skip files in already deleted folders
      if (!await file.exists()) continue;
      final relativePath = file.path.substring(loadedResourcesDir.length + 1);
      var nonRemovable = false;
      for (var resource in nonRemovableResources) {
        // relativePath.startsWith(resource): if we want to preserve a folder resource
        // resource.startsWith(relativePath): if we want to preserve a file resource
        if (relativePath.startsWith(resource) ||
            resource.startsWith(relativePath)) {
          nonRemovable = true;
          break;
        }
      }
      if (nonRemovable) continue;
      if (atLeastDaysOld > 0) {
        var stat = await file.stat();
        if (DateTime.now().difference(stat.modified).inDays < atLeastDaysOld) {
          continue;
        }
      }
      await file.delete(recursive: true);
    }
  }

  Future<void> purgeOutdatedCache(int atLeastDaysOld) async {
    var currentResources = <String>[];
    for (var r in _resourcesMap.values) {
      if (!r.startsWith(loadedResourcesDir)) continue;
      currentResources.add(r.substring(loadedResourcesDir.length + 1));
    }
    return deleteLoadedResources(currentResources, atLeastDaysOld);
  }

  Future<void> cache(List<String> resources, void Function(double) reportProgress, bool purgeOldCache) async {
    final resourcesToDownload = <String>[];
    _resourcesMap = {};

    for (final resource in resources) {
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
    await _download(resourcesToDownload, reportProgress);

    if (purgeOldCache) {
      const atLeastDaysOld = 30 * 9;
      await purgeOutdatedCache(atLeastDaysOld);
    }
  }

  bool isResourceAnArchive(String resource) {
    return resource.endsWith(ArchiveCacheHelper.extension);
  }

  Future<void> _download(List<String> resources, void Function(double) reportProgress) async {
    var progress = 0.0;
    for (var url in resources) {
      if (isResourceAnArchive(url)) {
        _resourcesMap[url] = await archiveCacheHelper.get(url, true);
      } else {
        _resourcesMap[url] = await fileCacheHelper.get(url, true);
      }

      progress += 1.0 / resources.length;
      reportProgress(progress);
    }
  }

}