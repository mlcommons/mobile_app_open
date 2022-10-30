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

  void Function()? onConfigChange;

  Map<String, TaskConfigDescription> configList = {};

  String currentConfigName = '';
  String currentConfigFilePath = '';
  late pb.MLPerfConfig currentConfig;

  ConfigManager._({
    required this.applicationDirectory,
    required this.resourceManager,
  });

  String get _defaultConfigFile => '$applicationDirectory/$_configListFileName';

  static Future<ConfigManager> create({
    required String applicationDirectory,
    required ResourceManager resourceManager,
  }) async {
    final result = ConfigManager._(
      applicationDirectory: applicationDirectory,
      resourceManager: resourceManager,
    );
    await result.initConfigList();
    return result;
  }

  void setUpdateNotifier(void Function() callback) {
    onConfigChange = callback;
  }

  Future<void> initConfigList() async {
    try {
      configList = await readConfigList(_defaultConfigFile);
    } catch (e, t) {
      print('error while reading config list: $e');
      print(t);
    }

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
    if (!await file.exists()) {
      return {};
    }
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
  /// currentConfig must not be read until this function has finished successfully
  Future<void> setConfig({required String name}) async {
    if (name == '') {
      name = defaultConfig.name;
    }
    final TaskConfigDescription? config = configList[name];
    if (config == null) {
      throw 'config with name $name not found';
    }
    currentConfigName = name;
    currentConfig = await readConfig(config);
    onConfigChange?.call();
  }

  Future<pb.MLPerfConfig> readConfig(TaskConfigDescription config) async {
    String configContent = await getFileContent(config.path);
    return getMLPerfConfig(configContent);
  }

  // TODO this should be responsibility of resource manager
  Future<String> getFileContent(String uri) async {
    if (isInternetResource(uri)) {
      currentConfigFilePath =
          await resourceManager.cacheManager.fileCacheHelper.get(uri, true);
      return await File(currentConfigFilePath).readAsString();
    }

    if (isAsset(uri)) {
      currentConfigFilePath = uri;
      final assetPath = stripAssetPrefix(uri);
      return await rootBundle.loadString(assetPath);
    }

    currentConfigFilePath = resourceManager.get(uri);
    if (!await File(currentConfigFilePath).exists()) {
      throw 'local config file is missing: $currentConfigFilePath';
    }
    return await File(currentConfigFilePath).readAsString();
  }
}
