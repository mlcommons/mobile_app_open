import 'dart:convert';
import 'dart:io';

import 'package:flutter/services.dart';

import 'package:mlperfbench/backend/bridge/ffi_config.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource_manager.dart';
import 'utils.dart';

const _configListFileName = 'benchmarksConfigurations.json';
const _defaultConfigName = 'default';
const _defaultConfigUrl = 'asset://assets/tasks.pbtxt';

class TaskConfigDescription {
  final String name;
  final String path;

  TaskConfigDescription(this.name, this.path);

  String getType(AppLocalizations stringResources) => isInternetResource(path)
      ? stringResources.settingsTaskConfigInternetResource
      : stringResources.settingsTaskConfigLocalResource;

  Map<String, String> asMap() => {name: path};
}

class ConfigManager {
  final String applicationDirectory;
  final ResourceManager resourceManager;
  final TaskConfigDescription defaultConfig =
      TaskConfigDescription(_defaultConfigName, _defaultConfigUrl);

  Map<String, TaskConfigDescription> configList = {};

  String configLocation = '';
  late pb.MLPerfConfig decodedConfig;

  ConfigManager._(this.applicationDirectory, this.resourceManager);

  String get _defaultConfigFile => '$applicationDirectory/$_configListFileName';

  static Future<ConfigManager> create(
    String applicationDirectory,
    ResourceManager resourceManager,
  ) async {
    final result = ConfigManager._(applicationDirectory, resourceManager);
    await result.initConfigList();
    return result;
  }

  Future<void> initConfigList() async {
    configList = await readConfigList(_defaultConfigFile);

    if (configList[defaultConfig.name] == null ||
        configList[defaultConfig.name]!.path != defaultConfig.path) {
      configList[defaultConfig.name] = defaultConfig;
      await updateConfigOnDisk();
    }
  }

  Future<void> updateConfigOnDisk() async {
    await writeConfigList(configList, _defaultConfigFile);
  }

  static Future<Map<String, TaskConfigDescription>> readConfigList(
      String configFile) async {
    final file = File(configFile);
    final jsonContent =
        jsonDecode(await file.readAsString()) as Map<String, dynamic>;

    final result = <String, TaskConfigDescription>{};
    for (final e in jsonContent.entries) {
      result[e.key] = TaskConfigDescription(e.key, e.value as String);
    }
    return result;
  }

  static Future<void> writeConfigList(
      Map<String, TaskConfigDescription> list, String configFile) async {
    final Map<String, dynamic> json = {};
    for (var e in list.entries) {
      json[e.key] = e.value.path;
    }

    const jsonEncoder = JsonEncoder.withIndent('  ');

    final file = File(configFile);
    await file.writeAsString(jsonEncoder.convert(json));
  }

  /// Can throw.
  /// decodedConfig must not be read until this function has finished successfully
  Future<void> loadConfig(String name) async {
    final TaskConfigDescription? config = configList[name];
    if (config == null) {
      throw 'config with name $name not found';
    }
    String configContent;
    if (isInternetResource(config.path)) {
      configLocation = await resourceManager.cacheManager.fileCacheHelper
          .get(config.path, true);
      configContent = await File(configLocation).readAsString();
    } else if (isAsset(config.path)) {
      configLocation = config.path;
      final assetPath = stripAssetPrefix(config.path);
      configContent = await rootBundle.loadString(assetPath);
    } else {
      configLocation = resourceManager.get(config.path);
      if (!await File(configLocation).exists()) {
        throw 'local config file is missing: $configLocation';
      }
      configContent = await File(configLocation).readAsString();
    }

    final nonRemovableResources = <String>[];
    if (isInternetResource(config.path)) {
      nonRemovableResources.add(resourceManager.cacheManager.fileCacheHelper
          .getResourceRelativePath(config.path));
    }

    decodedConfig = getMLPerfConfig(configContent);
  }
}
