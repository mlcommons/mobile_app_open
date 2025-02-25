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
      return 'Data folder must not be empty';
    }
    if (!await Directory(dataFolderPath).exists()) {
      return 'Data folder does not exist';
    }
    final resources = benchmarkStore.listResources(
      modes: selectedRunModes,
      benchmarks: benchmarkStore.activeBenchmarks,
    );
    final result = await resourceManager.validateResourcesExist(resources);
    final missing = result[false] ?? [];
    if (missing.isEmpty) return '';

    return errorDescription +
        missing.mapIndexed((i, element) => '\n${i + 1}) $element').join();
  }

  Future<String> validateChecksum(String errorDescription) async {
    final resources = benchmarkStore.listResources(
      modes: selectedRunModes,
      benchmarks: benchmarkStore.activeBenchmarks,
    );
    final checksumFailed =
        await resourceManager.validateResourcesChecksum(resources);
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
      Benchmark benchmark, BenchmarkRunMode mode) async {
    final resources = benchmarkStore.listResources(
      modes: [mode],
      benchmarks: [benchmark],
    );
    final result = await resourceManager.validateResourcesExist(resources);
    return result;
  }
}
