import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'utils.dart';

const _configurationsFileName = 'benchmarksConfigurations.json';
const _defaultConfigUrl =
    'https://raw.githubusercontent.com/mlcommons/mobile_models/main/v1_0/assets/tasks_v3.pbtxt';

class BenchmarksConfig {
  final String name;
  final String path;

  BenchmarksConfig(this.name, this.path);

  String getType(AppLocalizations stringResources) => isInternetResource(path)
      ? stringResources.internetResource
      : stringResources.localResource;
}

class ConfigurationsManager {
  final String applicationDirectory;
  final ResourceManager resourceManager;
  final BenchmarksConfig defaultConfig =
      BenchmarksConfig('default', _defaultConfigUrl);
  String currentConfigName;
  String configPath = '';

  ConfigurationsManager(
      this.applicationDirectory, this.currentConfigName, this.resourceManager);

  Future<BenchmarksConfig?> get currentConfig async =>
      await getConfig(currentConfigName);

  Future<void> setConfig(BenchmarksConfig config) async {
    if (isInternetResource(config.path)) {
      configPath = await resourceManager.cacheManager.fileCacheHelper
          .get(config.path, true);
    } else {
      configPath = resourceManager.get(config.path);
      if (!await File(configPath).exists()) {
        throw 'local config file is missing: $configPath';
      }
    }

    if (currentConfigName == config.name) {
      return;
    }

    currentConfigName = config.name;

    final nonRemovableResources = <String>[];
    if (isInternetResource(config.path)) {
      nonRemovableResources.add(resourceManager.cacheManager.fileCacheHelper
          .getResourceRelativePath(config.path));
    }

    await resourceManager.cacheManager
        .deleteLoadedResources(nonRemovableResources);
  }

  Future<File> createConfigurationsFile() async {
    final file = File('$applicationDirectory/$_configurationsFileName');
    if (await file.exists()) return file;

    print('Create ' + file.path);
    var config = <String, String>{defaultConfig.name: defaultConfig.path};
    final jsonEncoder = JsonEncoder.withIndent('  ');
    await file.writeAsString(jsonEncoder.convert(config));
    return file;
  }

  Future<Map<String, dynamic>> _readConfigs() async {
    final file = await createConfigurationsFile();
    return jsonDecode(await file.readAsString()) as Map<String, dynamic>;
  }

  Future<List<BenchmarksConfig>> getList() async {
    final jsonContent = await _readConfigs();

    final result = <BenchmarksConfig>[];
    for (final e in jsonContent.entries) {
      result.add(BenchmarksConfig(e.key, e.value as String));
    }
    return result;
  }

  Future<void> deleteDefaultConfig() async {
    final fileName = defaultConfig.path.split('/').last;
    var configFile = File('$applicationDirectory/$fileName');
    if (await configFile.exists()) {
      await configFile.delete();
    }
  }

  Future<BenchmarksConfig?> getConfig(String name) async {
    final jsonContent = await _readConfigs();
    final configPath = jsonContent[name] as String?;

    if (configPath != null) {
      return BenchmarksConfig(name, configPath);
    }
    return null;
  }
}
