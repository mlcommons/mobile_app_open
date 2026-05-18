import 'dart:io';

import 'package:collection/collection.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/benchmark/run_mode.dart';
import 'package:mlperfbench/resources/resource_manager.dart';
import 'package:mlperfbench/resources/utils.dart';

class ValidationHelper {
  final ResourceManager resourceManager;
  final BenchmarkStore benchmarkStore;
  final List<BenchmarkRunMode> selectedRunModes;

  ValidationHelper({
    required this.resourceManager,
    required this.benchmarkStore,
    required this.selectedRunModes,
  });

  Future<String> validateExternalResourcesDirectory(
      String errorDescription) async {
    final dataFolderPath = resourceManager.getDataFolder();
    if (dataFolderPath.isEmpty) {
      return 'Data folder must not be empty'; // TODO l10n
    }
    if (!await Directory(dataFolderPath).exists()) {
      return 'Data folder does not exist'; // TODO l10n
    }
    final resources = benchmarkStore.listResources(
      modes: selectedRunModes,
      benchmarks: benchmarkStore.activeBenchmarks,
    );
    if (await resourceManager.validateAllResourcesExist(resources)) return '';

    return errorDescription; // + missing.mapIndexed((i, element) => '\n${i + 1}) $element').join();
  }

  // TODO progress could be added here for verification dialog
  Future<String> validateChecksum(String errorDescription) async {
    final resources = benchmarkStore.listResources(
      modes: selectedRunModes,
      benchmarks: benchmarkStore.activeBenchmarks,
    );
    final checksumFailed =
        await resourceManager.validateResourcesChecksum(resources, (_,__){});
    if (checksumFailed.isEmpty) return '';
    final mismatchedPaths = checksumFailed.map((e) => '\n${e.path}').join();
    return errorDescription + mismatchedPaths;
  }

  Future<String> validateOfflineMode(String errorDescription) async {
    final resources = benchmarkStore.listResources(
      modes: selectedRunModes,
      benchmarks: benchmarkStore.activeBenchmarks,
    );
    final internetResources = filterInternetResources(resources);
    if (internetResources.isEmpty) return '';

    return errorDescription +
        internetResources
            .mapIndexed((i, element) => '\n${i + 1}) $element')
            .join();
  }

  Future<Map<bool, List<String>>> validateResourcesExist(
      Benchmark benchmark, BenchmarkRunMode mode) {
    final resources = benchmarkStore.listResources(
      modes: [mode],
      benchmarks: [benchmark],
    );
    return resourceManager.validateResourcesExist(resources);
  }

  Future<bool> validateAllResourcesExist(Benchmark benchmark,
      {BenchmarkRunMode? mode, List<BenchmarkRunMode> modes = const []}) {
    final resources = benchmarkStore.listResources(
      modes: mode != null ? [mode] : modes,
      benchmarks: [benchmark],
    );
    return resourceManager.validateAllResourcesExist(resources);
  }
}
