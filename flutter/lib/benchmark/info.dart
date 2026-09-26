import 'package:flutter/material.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'package:mlperfbench/ui/icons.dart';

class BenchmarkLocalizationInfo {
  final String name;
  final String detailsTitle;
  final String detailsContent;

  BenchmarkLocalizationInfo({
    required this.name,
    required this.detailsTitle,
    required this.detailsContent,
  });
}

class BenchmarkInfo {
  final pb.TaskConfig task;

  /// 'Object Detection', 'Image Classification (offline)', and so on.
  String get taskName => task.name;

  BenchmarkInfo(this.task);

  BenchmarkLocalizationInfo getLocalizedInfo(AppLocalizations stringResources) {
    switch (task.id) {
      // Every LLM variant shares one description; the size and the eval it
      // runs are already in the task's own name.
      case (BenchmarkId.llm1b):
      case (BenchmarkId.llm1bInstruct):
      case (BenchmarkId.llm3b):
      case (BenchmarkId.llm3bInstruct):
      case (BenchmarkId.llm8b):
      case (BenchmarkId.llm8bInstruct):
        return BenchmarkLocalizationInfo(
          name: task.name,
          detailsTitle: stringResources.benchInfoLlm,
          detailsContent: stringResources.benchInfoLlmDesc,
        );
      case (BenchmarkId.imageClassificationV2):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageClassification,
          detailsTitle: stringResources.benchInfoImageClassification,
          detailsContent: stringResources.benchInfoImageClassificationV2Desc,
        );
      case (BenchmarkId.imageClassificationOfflineV2):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageClassificationOffline,
          detailsTitle: stringResources.benchInfoImageClassification,
          detailsContent: stringResources.benchInfoImageClassificationV2Desc,
        );
      case (BenchmarkId.objectDetection):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameObjectDetection,
          detailsTitle: stringResources.benchInfoObjectDetection,
          detailsContent: stringResources.benchInfoObjectDetectionDesc,
        );
      case (BenchmarkId.imageSegmentationV2):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageSegmentation,
          detailsTitle: stringResources.benchInfoImageSegmentation,
          detailsContent: stringResources.benchInfoImageSegmentationDesc,
        );
      case (BenchmarkId.naturalLanguageProcessing):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameLanguageProcessing,
          detailsTitle: stringResources.benchInfoLanguageProcessing,
          detailsContent: stringResources.benchInfoLanguageProcessingDesc,
        );
      case (BenchmarkId.superResolution):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameSuperResolution,
          detailsTitle: stringResources.benchInfoSuperResolution,
          detailsContent: stringResources.benchInfoSuperResolutionDesc,
        );
      case (BenchmarkId.stableDiffusion):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameStableDiffusion,
          detailsTitle: stringResources.benchInfoStableDiffusion,
          detailsContent: stringResources.benchInfoStableDiffusionDesc,
        );
      default:
        throw 'unhandled task id: ${task.id}';
    }
  }

  bool get isOffline => task.scenario == 'Offline';

  double get maxThroughput => task.maxThroughput;

  Widget get icon => BenchmarkIcons.getDarkIcon(task.id);

  Widget get iconWhite => BenchmarkIcons.getLightIcon(task.id);

  @override
  String toString() => 'Benchmark:${task.id}';
}

/// Name, icon and description of a whole task set.
///
/// A set is a frontend-only grouping, so it needs its own identity rather than
/// borrowing one from whichever benchmark happens to sort first.
class BenchmarkSetInfo {
  final pb.TaskSet config;

  BenchmarkSetInfo(this.config);

  String get name => config.name;

  Widget get icon => BenchmarkIcons.getSetDarkIcon(config.id);

  /// Null when the set has no description of its own. Sets come from config,
  /// so an unknown one is a gap to fall back from, not a crash.
  String? localizedDetails(AppLocalizations stringResources) {
    switch (config.id) {
      case (BenchmarkSetId.llm):
        return stringResources.benchInfoLlmDesc;
      case (BenchmarkSetId.imageClassification):
        return stringResources.benchInfoImageClassificationV2Desc;
      default:
        return null;
    }
  }
}
