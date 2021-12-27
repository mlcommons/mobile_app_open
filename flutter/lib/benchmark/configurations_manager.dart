import 'dart:convert';
import 'dart:io';

import 'benchmark.dart';

const _configurationsFileName = 'benchmarksConfigurations.json';

class ConfigurationsManager {
  final String applicationDirectory;
  final BenchmarksConfiguration defaultBenchmarksConfiguration =
      BenchmarksConfiguration(
    'default',
    'https://raw.githubusercontent.com/mlcommons/mobile_models/main/v1_0/assets/tasks_v3.pbtxt',
  );

  ConfigurationsManager(this.applicationDirectory);

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

  Future<void> deleteDefaultBenchmarksConfiguration() async {
    final fileName = defaultBenchmarksConfiguration.path.split('/').last;
    var configFile = File('$applicationDirectory/$fileName');
    if (await configFile.exists()) {
      await configFile.delete();
    }
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
}
