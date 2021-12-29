import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'package:path_provider/path_provider.dart';
import 'package:yaml/yaml.dart';

import 'cache_manager.dart';
import 'configurations_manager.dart';
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
  late final ConfigurationsManager configurationsManager;
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

  void handleResources(List<String> resources, bool purgeOldCache) async {
    _done = false;
    _onUpdate();

    var internetResources = <String>[];
    for (final resource in resources) {
      if (resource.startsWith(_applicationDirectoryPrefix)) continue;
      if (isInternetResource(resource)) {
        internetResources.add(resource);
        continue;
      }
      throw 'forbidden path: $resource (only http://, https:// and app:// resources are allowed)';
    }

    await cacheManager.cache(internetResources, (double val) {
      _progressString = '${(val * 100).round()}%';
      _onUpdate();
    }, purgeOldCache);

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
    configurationsManager = ConfigurationsManager(applicationDirectory);
    resultManager = ResultManager(applicationDirectory);
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

  Future<File> moveFile(File source, String destination) async {
    await Directory(destination).parent.create(recursive: true);
    try {
      return await source.rename(destination);
    } on FileSystemException {
      var result = await source.copy(destination);
      await source.delete();
      return result;
    }
  }
}
