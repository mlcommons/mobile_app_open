import 'dart:collection';
import 'dart:convert';
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'package:path_provider/path_provider.dart';
import 'package:wakelock/wakelock.dart';
import 'package:yaml/yaml.dart';

import 'package:mlcommons_ios_app/benchmark/benchmark.dart';
import 'package:mlcommons_ios_app/benchmark/cache_manager.dart';

bool isInternetResource(String uri) =>
    uri.startsWith('http://') || uri.startsWith('https://');

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
  static const _zipPattern = '.zip';
  static const _applicationDirectoryPrefix = 'app://';
  static const _configurationsFileName = 'benchmarksConfigurations.json';
  static const _jsonResultFileName = 'result.json';
  static const _loadedResourcesDirName = 'loaded_resources';

  final VoidCallback onUpdate;
  final BenchmarksConfiguration defaultBenchmarksConfiguration =
      BenchmarksConfiguration(
    'default',
    'https://raw.githubusercontent.com/mlcommons/mobile_models/main/v1_0/assets/tasks_v3.pbtxt',
  );

  Map<String, String> _resourcesMap = {};
  bool _done = false;
  List<String> _resources = [];
  double _progress = 0;

  late final String applicationDirectory;
  late final String loadedResourcesDir;
  late final String _jsonResultPath;

  late final List<BatchPreset> _batchPresets;

  late final CacheManager cacheManager;

  ResourceManager(this.onUpdate);

  bool get done => _done;

  String get progress {
    return '${(_progress * 100).round()}%';
  }

  String get(String uri) {
    if (uri.isEmpty) return '';

    if (uri.startsWith(_applicationDirectoryPrefix)) {
      final resourceSystemPath =
          uri.replaceFirst(_applicationDirectoryPrefix, applicationDirectory);
      return resourceSystemPath;
    }

    return _resourcesMap[uri]!;
  }

  Future<bool> isResourceExist(String? uri) async {
    if (uri == null) return false;

    final path = get(uri);

    return path == '' ||
        await File(path).exists() ||
        await Directory(path).exists();
  }

  Future<BenchmarksConfiguration?> getChosenConfiguration(
      String chosenBenchmarksConfigurationName) async {
    final file = File('$applicationDirectory/$_configurationsFileName');
    if (await file.exists()) {
      final content = await file.readAsString();
      final jsonContent = jsonDecode(content) as Map<String, dynamic>;
      final configPath =
          jsonContent[chosenBenchmarksConfigurationName] as String?;

      if (configPath != null) {
        return BenchmarksConfiguration(
            chosenBenchmarksConfigurationName, configPath);
      }
    }
    if (chosenBenchmarksConfigurationName !=
        defaultBenchmarksConfiguration.name) {
      return null;
    }

    return defaultBenchmarksConfiguration;
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

    loadedResourcesDir = '$applicationDirectory/$_loadedResourcesDirName';
    cacheManager = CacheManager(loadedResourcesDir);
    await cacheManager.init();
    _jsonResultPath = '$applicationDirectory/$_jsonResultFileName';

    await Directory(loadedResourcesDir).create();
  }

  Future<void> createConfigurationFile() async {
    final file = File('$applicationDirectory/$_configurationsFileName');
    if (!await file.exists()) {
      print('Create ' + file.path);
      var config = <String, String>{
        defaultBenchmarksConfiguration.name: defaultBenchmarksConfiguration.path
      };
      await file.writeAsString(jsonEncode(config));
    }
  }

  Future<void> writeToJsonResult(List<Map<String, dynamic>> content) async {
    final jsonFile = File(_jsonResultPath);

    final jsonEncoder = JsonEncoder.withIndent('  ');
    var encoded = jsonEncoder.convert(content);
    // needed to match android behavior
    encoded = encoded.replaceAll('/', '\\/');
    await jsonFile.writeAsString(encoded);
  }

  Future<String> get jsonResult async {
    final file = File(_jsonResultPath);

    if (await file.exists()) {
      return file.readAsString();
    }

    return '';
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

  Future<void> deleteDefaultBenchmarksConfiguration() async {
    final fileName = defaultBenchmarksConfiguration.path.split('/').last;
    var configFile = File('$applicationDirectory/$fileName');
    if (await configFile.exists()) {
      await configFile.delete();
    }
  }

  Future<void> deleteResultJson() async {
    var resultsFile = File(_jsonResultPath);
    if (await resultsFile.exists()) {
      await resultsFile.delete();
    }
  }

  void handleResources(bool purgeOldCache) async {
    // disable screen sleep when benchmarks is running
    await Wakelock.enable();

    final resourcesFromInternet = ListQueue<String>();
    _resourcesMap = {};
    _progress = 0;
    _done = false;
    onUpdate();

    for (final resource in _resources) {
      if (resource.startsWith(_applicationDirectoryPrefix)) continue;
      if (_resourcesMap.containsKey(resource)) continue;
      if (resourcesFromInternet.contains(resource)) continue;

      if (isInternetResource(resource)) {
        var filename = cacheManager.getResourceRelativePath(resource);

        if (!await cacheManager.isResourceCached(resource)) {
          resourcesFromInternet.add(resource);
        } else {
          _resourcesMap[resource] = await _getResourceFromFileSystem(filename, loadedResourcesDir);
        }

        continue;
      }

      throw 'Could not get file path for $resource';
    }
    await _handleResourcesFromInternet(resourcesFromInternet);

    _done = true;
    onUpdate();

    await Wakelock.disable();

    if (purgeOldCache) {
      const atLeastDaysOld = 30 * 9;
      await purgeOutdatedCache(atLeastDaysOld);
    }
  }

  set resources(List<String> resources) => _resources = resources;

  Future<void> _handleResourcesFromInternet(Queue<String> resourcesPath) async {
    final downloadedResourcesCount = resourcesPath.length;

    for (var res in resourcesPath) {
      var cachePath = cacheManager.getCachePath(res);
      if (cacheManager.isResourceAnArchive(cachePath)) {
        _resourcesMap[res] = await cacheManager.getArchive(res, true);
      } else {
        _resourcesMap[res] = await cacheManager.getFile(res, true);
      }

      _progress += 1 / downloadedResourcesCount;
      onUpdate();
    }
  }

  Future<String> _getResourceFromFileSystem(
      String relativeResourcePath, String baseDirectoryPath) async {
    final filePath = '$baseDirectoryPath/$relativeResourcePath';

    if (await File(filePath).exists()) {
      return filePath;
    }

    if (cacheManager.isResourceAnArchive(relativeResourcePath)) {
      final directory = cacheManager.getArchiveFolder(relativeResourcePath);
      final directoryPath = '$baseDirectoryPath/$directory';
      if (await Directory(directoryPath).exists()) {
        return directoryPath;
      }
    }

    return '';
  }

  Future<List<BenchmarksConfiguration>>
      getAvailableBenchmarksConfigurations() async {
    final benchmarksConfigurations = <BenchmarksConfiguration>[];
    final file = File('$applicationDirectory/$_configurationsFileName');
    if (!await file.exists()) {
      await createConfigurationFile();
    }
    assert(await file.exists());

    final content =
        jsonDecode(await file.readAsString()) as Map<String, dynamic>;
    for (final benchmarksConfigurationContent in content.entries) {
      final benchmarksConfiguration = BenchmarksConfiguration(
          benchmarksConfigurationContent.key,
          benchmarksConfigurationContent.value as String);
      benchmarksConfigurations.add(benchmarksConfiguration);
    }
    return benchmarksConfigurations;
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

    final yamlString =
        await rootBundle.loadString('assets/default_batch_settings.yaml');
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

  Future<Directory> moveDirectory(Directory source, String destination) async {
    await Directory(destination).parent.create(recursive: true);
    try {
      return await source.rename(destination);
    } catch (e) {
      await _copyDirectory(source, destination);
      await source.delete(recursive: true);
      return Directory(destination);
    }
  }

  Future<void> _copyDirectory(Directory source, String destination) async {
    var sourcePrefixLength = source.path.length;
    if (!(source.path.endsWith('/') || source.path.endsWith('\\'))) {
      sourcePrefixLength += 1;
    }

    await for (var e in source.list(recursive: true)) {
      var stat = await e.stat();
      final relativePath = e.path.substring(sourcePrefixLength);
      final entityDestination = '$destination/$relativePath';
      switch (stat.type) {
        case FileSystemEntityType.file:
          var file = File(e.path);
          await File(entityDestination).parent.create(recursive: true);
          await file.copy(entityDestination);
          break;
        case FileSystemEntityType.directory:
          await _copyDirectory(Directory(e.path), entityDestination);
          break;
        case FileSystemEntityType.link:
          throw "moving links isn't supported";
        case FileSystemEntityType.notFound:
          throw 'unexpected missing file';
        default:
          throw 'unexpected file type';
      }
    }
  }
}
