import 'dart:convert';
import 'dart:io';

import 'package:mlcommons_ios_app/localizations/app_localizations.dart';
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
  final BenchmarksConfig defaultConfig =
      BenchmarksConfig('default', _defaultConfigUrl);

  ConfigurationsManager(this.applicationDirectory);

  Future<File> createConfigurationFile() async {
    final file = File('$applicationDirectory/$_configurationsFileName');
    if (await file.exists()) return file;

    print('Create ' + file.path);
    var config = <String, String>{defaultConfig.name: defaultConfig.path};
    final jsonEncoder = JsonEncoder.withIndent('  ');
    await file.writeAsString(jsonEncoder.convert(config));
    return file;
  }

  Future<Map<String, dynamic>> _readConfigs() async {
    final file = await createConfigurationFile();
    return jsonDecode(await file.readAsString()) as Map<String, dynamic>;
  }

  Future<List<BenchmarksConfig>> getAvailableConfigs() async {
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

  Future<BenchmarksConfig?> getChosenConfig(String name) async {
    final jsonContent = await _readConfigs();
    final configPath = jsonContent[name] as String?;

    if (configPath != null) {
      return BenchmarksConfig(name, configPath);
    }
    return null;
  }
}
