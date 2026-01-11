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
      case (BenchmarkId.llm):
      // TODO translate this and add proper info
        return BenchmarkLocalizationInfo(
            name: 'LLM',
            detailsTitle: 'Large Language Model',
            detailsContent: 'you know what ChatGPT is...');
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
