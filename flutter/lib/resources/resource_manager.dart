import 'dart:io';

import 'package:flutter/foundation.dart';

import 'package:crypto/crypto.dart';
import 'package:path_provider/path_provider.dart';

import 'package:mlperfbench/resources/cache_manager.dart';
import 'package:mlperfbench/resources/resource.dart';
import 'package:mlperfbench/resources/result_manager.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/store.dart';

class ResourceManager {
  static const _dataPrefix = 'local://';
  static const _loadedResourcesDirName = 'loaded_resources';

  final VoidCallback _onUpdate;
  final Store store;

  bool _done = false;
  String _loadingPath = '';
  double _loadingProgress = 0.0;

  late final String applicationDirectory;
  late final String _loadedResourcesDir;

  late final CacheManager cacheManager;
  late final ResultManager resultManager;

  ResourceManager(this._onUpdate, this.store);

  bool get done => _done;

  double get loadingProgress {
    return _loadingProgress;
  }

  String get loadingPath {
    return _loadingPath;
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
    return applicationDirectory;
  }

  String getDataPrefix() {
    return _dataPrefix;
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
    _loadingPath = '';
    _loadingProgress = 0.0;
    _done = false;
    _onUpdate();

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
    await cacheManager.cache(internetPaths,
        (double currentProgress, String currentPath) {
      _loadingProgress = currentProgress;
      _loadingPath = currentPath;
      _onUpdate();
    }, purgeOldCache);

    final checksumFailed = await validateResourcesChecksum(resources);
    if (checksumFailed.isNotEmpty) {
      final mismatchedPaths = checksumFailed.map((e) => '\n${e.path}').join();
      throw 'Checksum validation failed for: $mismatchedPaths';
    }

    // delete downloaded archives to free up disk space
    await cacheManager.deleteArchives(internetPaths);

    _done = true;
    _onUpdate();
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

  Future<void> initSystemPaths() async {
    applicationDirectory = await getApplicationDirectory();
    await Directory(applicationDirectory).create(recursive: true);
    _loadedResourcesDir = '$applicationDirectory/$_loadedResourcesDirName';
    await Directory(_loadedResourcesDir).create();

    cacheManager = CacheManager(_loadedResourcesDir);
    resultManager = await ResultManager.create(applicationDirectory);
  }

  Future<List<String>> validateResourcesExist(List<Resource> resources) async {
    final missingResources = <String>[];
    for (var r in resources) {
      if (!await isResourceExist(r.path)) {
        final resolvedPath = get(r.path);
        missingResources.add(resolvedPath);
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
