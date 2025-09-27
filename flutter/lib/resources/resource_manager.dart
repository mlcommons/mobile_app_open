import 'dart:io';

import 'package:flutter/foundation.dart';

import 'package:crypto/crypto.dart';
import 'package:path_provider/path_provider.dart';

import 'package:mlperfbench/resources/cache_manager.dart';
import 'package:mlperfbench/resources/resource.dart';
import 'package:mlperfbench/resources/result_manager.dart';
import 'package:mlperfbench/resources/utils.dart';
import 'package:mlperfbench/store.dart';

enum ResourceLoadingStatus {
  done,
  loading,
  verifying,
}

class ResourceManager {
  static const _dataPrefix = 'local://';
  static const _loadedResourcesDirName = 'loaded_resources';
  static const _symlinksDirName = 'symlinks';

  final VoidCallback _onUpdate;
  final Store store;

  // We start out as loading to prevent benchmarks from running before handleResources() is called.
  ResourceLoadingStatus _status = ResourceLoadingStatus.loading;
  String _loadingPath = '';
  double _loadingProgress = 0.0;

  late final String applicationDirectory;
  late final String _loadedResourcesDir;

  late final CacheManager cacheManager;
  late final ResultManager resultManager;

  ResourceManager(this._onUpdate, this.store);

  ResourceLoadingStatus get status {
    return _status;
  }

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
      final resourceSystemPath = cacheManager.get(uri);
      return resourceSystemPath ?? '';
    }
    if (File(uri).isAbsolute) {
      return uri;
    }

    throw 'invalid resource path: $uri';
  }

  // Creates symlinks to the given file paths and put them in one cache directory.
  Future<String> getModelPath(List<String> paths, String dirName) async {
    String modelPath;
    if (paths.isEmpty) {
      throw 'List of URIs cannot be empty';
    }
    if (dirName.contains(' ')) {
      throw 'Directory name cannot contain spaces';
    }
    final cacheDir = await getApplicationCacheDirectory();
    final modelDir = Directory('${cacheDir.path}/$_symlinksDirName/$dirName');
    final files = paths.map((uri) => File(get(uri))).toList();
    final symlinks = await _createSymlinks(files, modelDir);
    if (paths.length == 1) {
      modelPath = symlinks.first;
    } else {
      modelPath = modelDir.path;
    }
    return modelPath;
  }

  String getDataFolder() {
    return applicationDirectory;
  }

  String getDataPrefix() {
    return _dataPrefix;
  }

  Future<bool> isChecksumMatched(String filePath, String md5Checksum) async {
    var fileStream = File(filePath).openRead();
    final checksum = (await md5.bind(fileStream).first).toString();
    return checksum == md5Checksum;
  }

  Future<void> handleResources({
    required List<Resource> resources,
    required bool purgeOldCache,
    required bool downloadMissing,
  }) async {
    _loadingPath = '';
    _loadingProgress = 0.001;
    _status = ResourceLoadingStatus.loading;
    _onUpdate();
    try {
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
      try {
        await cacheManager.cache(
          urls: internetPaths,
          onProgressUpdate: (double currentProgress, String currentPath) {
            _loadingProgress = currentProgress;
            _loadingPath = currentPath;
            _onUpdate();
          },
          purgeOldCache: purgeOldCache,
          downloadMissing: downloadMissing,
        );
      } on SocketException {
        throw 'A network error has occurred. Please make sure you are connected to the internet.';
      }

      // Make sure to only checksum resources that were downloaded in the step above
      final newResources = internetResources
          .where((element) => cacheManager.newDownloads.contains(element.path))
          .toList();

      _status = ResourceLoadingStatus.verifying;
      final checksumFailed = await validateResourcesChecksum(
        newResources,
        (double currentProgress, String currentPath) {
          _loadingProgress = currentProgress;
          _loadingPath = currentPath;
          _onUpdate();
        },
      );
      if (checksumFailed.isNotEmpty) {
        final checksumFailedPathString =
            checksumFailed.map((e) => '\n${e.path}').join();
        final checksumFailedPaths = checksumFailed.map((e) => e.path).toList();
        await cacheManager.deleteFiles(checksumFailedPaths);
        throw 'Checksum validation failed for: $checksumFailedPathString. \nPlease download the missing files again.';
      }
      // delete downloaded archives to free up disk space
      await cacheManager.deleteArchives(internetPaths);
    } finally {
      _loadingPath = '';
      _loadingProgress = 1.0;
      _status = ResourceLoadingStatus.done;
      _onUpdate();
    }
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

  // Returns a map of { true: [existedResources], false: [missingResources] }
  Future<Map<bool, List<String>>> validateResourcesExist(
      List<Resource> resources) async {
    final missingResources = <String>[];
    final existedResources = <String>[];
    for (var r in resources) {
      final resolvedPath = get(r.path);
      if (resolvedPath.isEmpty) {
        missingResources.add(r.path);
      } else {
        final isResourceExist = await File(resolvedPath).exists() ||
            await Directory(resolvedPath).exists();
        if (isResourceExist) {
          existedResources.add(r.path);
        } else {
          missingResources.add(r.path);
        }
      }
    }
    final result = {
      false: missingResources,
      true: existedResources,
    };
    return result;
  }


  // Similar to [validateResourcesExist], but returns one result for all resources provided
  Future<bool> validateAllResourcesExist(List<Resource> resources) async {
    for (Resource r in resources) {
      final resolvedPath = get(r.path);
      if (resolvedPath.isEmpty ||
          (!await File(resolvedPath).exists() &&
              !await Directory(resolvedPath).exists())) {
        return false;
      }
    }
    return true;
  }

  Future<List<Resource>> validateResourcesChecksum(List<Resource> resources,
      void Function(double, String) onProgressUpdate) async {
    final checksumFailedResources = <Resource>[];
    double progress = 0.0;
    for (final resource in resources) {
      progress += 0.1 / resources.length;
      onProgressUpdate(progress, resource.path);

      final md5Checksum = resource.md5Checksum;
      if (md5Checksum.isEmpty) continue;
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

      progress += 0.9 / resources.length;
      onProgressUpdate(progress, resource.path);
    }
    return checksumFailedResources;
  }

  Future<List<String>> _createSymlinks(
      List<File> files, Directory cacheDir) async {
    if (!await cacheDir.exists()) {
      await cacheDir.create(recursive: true);
    }
    List<String> symlinkPaths = [];
    for (final file in files) {
      final symlinkPath = '${cacheDir.path}/${file.uri.pathSegments.last}';
      symlinkPaths.add(symlinkPath);
      final link = Link(symlinkPath);
      if (await link.exists()) {
        await link.delete();
      }
      await link.create(file.path);
    }
    return symlinkPaths;
  }
}
