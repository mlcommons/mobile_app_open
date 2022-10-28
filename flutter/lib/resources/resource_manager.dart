import 'dart:io';

import 'package:flutter/foundation.dart';

import 'package:crypto/crypto.dart';
import 'package:path_provider/path_provider.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/settings/data_folder_type.dart';
import 'cache_manager.dart';
import 'resource.dart';
import 'result_manager.dart';
import 'utils.dart';

class ResourceManager {
  static const _dataPrefix = 'local://';
  static const _loadedResourcesDirName = 'loaded_resources';

  final Store store;
  final String resourceDir;
  final CacheManager cacheManager;
  final ResultManager resultManager;

  VoidCallback? _onUpdate;
  bool _done = false;
  String _progressString = '0%';

  ResourceManager({
    required this.store,
    required this.resourceDir,
    required this.cacheManager,
    required this.resultManager,
  });

  static Future<ResourceManager> create({
    required Store store,
    required String resourceDir,
    required ResultManager resultManager,
  }) async {
    final loadedResourcesDir = getLoadedResourcesDir(resourceDir);

    await Directory(resourceDir).create(recursive: true);
    await Directory(loadedResourcesDir).create();

    return ResourceManager(
      store: store,
      resourceDir: resourceDir,
      cacheManager: CacheManager(loadedResourcesDir),
      resultManager: resultManager,
    );
  }

  static String getLoadedResourcesDir(String appDir) {
    if (defaultCacheFolder.isNotEmpty) {
      return defaultCacheFolder;
    } else {
      return '$appDir/$_loadedResourcesDirName';
    }
  }

  void setUpdateNotifier(void Function() callback) {
    _onUpdate = callback;
  }

  bool get done => _done;

  String get progress {
    return _progressString;
  }

  String get(String uri) {
    if (uri.isEmpty) return '';

    if (uri.startsWith(_dataPrefix)) {
      final resourceSystemPath = uri.replaceFirst(_dataPrefix, getDataFolder());
      return resourceSystemPath;
    }
    if (isInternetResource(uri)) {
      return cacheManager.get(uri)!;
    }
    if (File(uri).isAbsolute) {
      return uri;
    }

    throw 'invalid resource path: $uri';
  }

  String getDataFolder() {
    switch (parseDataFolderType(store.dataFolderType)) {
      case DataFolderType.default_:
        if (defaultDataFolder.isNotEmpty) {
          return defaultDataFolder;
        } else {
          return resourceDir;
        }
      case DataFolderType.appFolder:
        return resourceDir;
      case DataFolderType.custom:
        return store.customDataFolder;
    }
  }

  Future<bool> isResourceExist(String? uri) async {
    if (uri == null) return false;

    final path = get(uri);

    return path == '' ||
        await File(path).exists() ||
        await Directory(path).exists();
  }

  Future<bool> isChecksumMatched(String filePath, String md5Checksum) async {
    var fileStream = File(filePath).openRead();
    final checksum = (await md5.bind(fileStream).first).toString();
    return checksum == md5Checksum;
  }

  Future<void> handleResources(
      List<Resource> resources, bool purgeOldCache) async {
    _progressString = '0%';
    _done = false;
    _onUpdate?.call();

    var internetResources = <Resource>[];
    for (final resource in resources) {
      if (resource.path.startsWith(_dataPrefix)) continue;
      if (isInternetResource(resource.path)) {
        internetResources.add(resource);
        continue;
      }
      throw 'forbidden path: ${resource.path} (only http://, https:// and local:// resources are allowed)';
    }

    final internetPaths = internetResources.map((e) => e.path).toList();
    await cacheManager.cache(internetPaths, (double val) {
      _progressString = '${(val * 100).round()}%';
      _onUpdate?.call();
    }, purgeOldCache);

    final checksumFailed = await validateResourcesChecksum(resources);
    if (checksumFailed.isNotEmpty) {
      final mismatchedPaths = checksumFailed.map((e) => '\n${e.path}').join();
      throw 'Checksum validation failed for: $mismatchedPaths';
    }

    // delete downloaded archives to free up disk space
    await cacheManager.deleteArchives(internetPaths);

    _done = true;
    _onUpdate?.call();
  }

  static Future<String> getApplicationDirectory() async {
    // applicationDirectory should be visible to user
    Directory? dir;
    if (Platform.isIOS) {
      dir = await getApplicationDocumentsDirectory();
    } else if (Platform.isAndroid) {
      dir = await getExternalStorageDirectory();
    } else if (Platform.isWindows) {
      dir = await getApplicationDocumentsDirectory();
      dir = Directory(dir.path.replaceAll('\\', '/'));
      dir = Directory('${dir.path}/MLCommons/MLPerfBench');
    } else {
      throw 'unsupported platform';
    }
    if (dir != null) {
      return dir.path;
    } else {
      return (await getApplicationDocumentsDirectory()).path;
    }
  }

  Future<List<String>> validateResourcesExist(List<Resource> resources) async {
    final missingResources = <String>[];
    for (var r in resources) {
      if (!await isResourceExist(r.path)) {
        missingResources.add(r.path);
      }
    }
    return missingResources;
  }

  Future<List<Resource>> validateResourcesChecksum(
      List<Resource> resources) async {
    final checksumFailedResources = <Resource>[];
    for (final resource in resources) {
      final md5Checksum = resource.md5Checksum;
      if (md5Checksum == null || md5Checksum.isEmpty) continue;
      String? localPath;
      if (cacheManager.isResourceAnArchive(resource.path)) {
        localPath = cacheManager.getArchive(resource.path);
      } else {
        localPath = cacheManager.get(resource.path);
      }
      if (localPath == null || !(await File(localPath).exists())) continue;
      if (!await isChecksumMatched(localPath, md5Checksum)) {
        checksumFailedResources.add(resource);
      }
    }
    return checksumFailedResources;
  }
}
