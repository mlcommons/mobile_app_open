import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench/backend/bridge/ffi_config.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/resources/resource_manager.dart';
import 'utils.dart';

const _configListFileName = 'benchmarksConfigurations.json';
const _defaultConfigName = 'default';
const _defaultConfigUrl =
    'https://raw.githubusercontent.com/mlcommons/mobile_models/main/v2_0/assets/tasks_flutterapp_v2.pbtxt';

class TaskConfigDescription {
  final String name;
  final String path;

  TaskConfigDescription(this.name, this.path);

  String getType(AppLocalizations stringResources) => isInternetResource(path)
      ? stringResources.internetResource
      : stringResources.localResource;

  Map<String, String> asMap() => {name: path};
}

class ConfigManager {
  final String applicationDirectory;
  final ResourceManager resourceManager;
  final TaskConfigDescription defaultConfig =
      TaskConfigDescription(_defaultConfigName, _defaultConfigUrl);

  String configPath = '';
  late pb.MLPerfConfig decodedConfig;

  ConfigManager(this.applicationDirectory, this.resourceManager);

  /// Can throw.
  /// decodedConfig must not be read until this function has finished successfully
  Future<void> loadConfig(String name) async {
    TaskConfigDescription? config;
    for (var c in await getConfigs()) {
      if (c.name == name) {
        config = c;
        break;
      }
    }
    if (config == null) {
      throw 'config with name $name not found';
    }
    if (isInternetResource(config.path)) {
      configPath = await resourceManager.cacheManager.fileCacheHelper
          .get(config.path, true);
    } else {
      configPath = resourceManager.get(config.path);
      if (!await File(configPath).exists()) {
        throw 'local config file is missing: $configPath';
      }
    }

    final nonRemovableResources = <String>[];
    if (isInternetResource(config.path)) {
      nonRemovableResources.add(resourceManager.cacheManager.fileCacheHelper
          .getResourceRelativePath(config.path));
    }

    decodedConfig = getMLPerfConfig(await File(configPath).readAsString());
  }

  Future<File> _createOrUpdateConfigListFile() async {
    final file = File('$applicationDirectory/$_configListFileName');
    final jsonEncoder = JsonEncoder.withIndent('  ');

    if (!await file.exists()) {
      print('Create new config file at ' + file.path);
      await file.writeAsString(jsonEncoder.convert(defaultConfig.asMap()));
      return file;
    }

    final configs =
        jsonDecode(await file.readAsString()) as Map<String, dynamic>;
    if (configs[defaultConfig.name] != defaultConfig.path) {
      print('Update default config path in ' + file.path);
      configs[defaultConfig.name] = defaultConfig.path;
      await file.writeAsString(jsonEncoder.convert(configs));
    }

    return file;
  }

  Future<List<TaskConfigDescription>> getConfigs() async {
    final file = await _createOrUpdateConfigListFile();
    final jsonContent =
        jsonDecode(await file.readAsString()) as Map<String, dynamic>;

    final result = <TaskConfigDescription>[];
    for (final e in jsonContent.entries) {
      result.add(TaskConfigDescription(e.key, e.value as String));
    }
    return result;
  }
}
