import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'package:crypto/crypto.dart';
import 'package:path_provider/path_provider.dart';
import 'package:yaml/yaml.dart';

import 'package:mlperfbench/device_info.dart';
import 'cache_manager.dart';
import 'resource.dart';
import 'result_manager.dart';
import 'utils.dart';

class BatchPreset {
  final String name;
  final int batchSize;
  final int shardsCount;

  BatchPreset({
    required this.name,
    required this.batchSize,
    required this.shardsCount,
  });
}

class ResourceManager {
  static const _applicationDirectoryPrefix = 'app://';
  static const _loadedResourcesDirName = 'loaded_resources';
  static const _defaultBatchSettingsPath = 'assets/default_batch_settings.yaml';

  final VoidCallback _onUpdate;

  bool _done = false;
  String _progressString = '0%';

  late final String applicationDirectory;
  late final String _loadedResourcesDir;

  late final List<BatchPreset> _batchPresets;

  late final CacheManager cacheManager;
  late final ResultManager resultManager;

  ResourceManager(this._onUpdate);

  bool get done => _done;

  String get progress {
    return _progressString;
  }

  String get(String uri) {
    if (uri.isEmpty) return '';

    if (uri.startsWith(_applicationDirectoryPrefix)) {
      final resourceSystemPath =
          uri.replaceFirst(_applicationDirectoryPrefix, applicationDirectory);
      return resourceSystemPath;
    }

    return cacheManager.get(uri)!;
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

  void handleResources(List<Resource> resources, bool purgeOldCache) async {
    _progressString = '0%';
    _done = false;
    _onUpdate();

    var internetResources = <Resource>[];
    for (final resource in resources) {
      if (resource.path.startsWith(_applicationDirectoryPrefix)) continue;
      if (isInternetResource(resource.path)) {
        internetResources.add(resource);
        continue;
      }
      throw 'forbidden path: ${resource.path} (only http://, https:// and app:// resources are allowed)';
    }

    final internetPaths = internetResources.map((e) => e.path).toList();
    await cacheManager.cache(internetPaths, (double val) {
      _progressString = '${(val * 100).round()}%';
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
      dir = await getDownloadsDirectory();
      dir = Directory(dir!.path.replaceAll('\\', '/'));
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
    _loadedResourcesDir = '$applicationDirectory/$_loadedResourcesDirName';
    await Directory(_loadedResourcesDir).create();

    cacheManager = CacheManager(_loadedResourcesDir);
    resultManager = ResultManager(applicationDirectory);
    await resultManager.init();
  }

  BatchPreset? getDefaultBatchPreset() {
    if (Platform.isAndroid) {
      // batch presets are disabled for android
      return null;
    }
    final presets = getBatchPresets();
    for (var preset in presets) {
      if (DeviceInfo.model.startsWith(preset.name)) {
        return preset;
      }
    }
    return presets[0];
  }

  List<BatchPreset> getBatchPresets() {
    return _batchPresets;
  }

  Future<void> loadBatchPresets() async {
    var result = <BatchPreset>[];
    result.add(BatchPreset(
      name: 'Default',
      batchSize: 2,
      shardsCount: 4,
    ));
    result.add(BatchPreset(
      name: 'Custom',
      batchSize: 0,
      shardsCount: 0,
    ));

    final yamlString = await rootBundle.loadString(_defaultBatchSettingsPath);
    for (var item in loadYaml(yamlString)['devices']) {
      result.add(BatchPreset(
        name: item['name'] as String,
        batchSize: item['batch-size'] as int,
        shardsCount: item['shards-count'] as int,
      ));
    }

    _batchPresets = result;
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
