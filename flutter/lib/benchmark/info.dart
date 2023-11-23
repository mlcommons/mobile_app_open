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
      case (BenchmarkId.imageClassification):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageClassification,
          detailsTitle: stringResources.benchInfoImageClassification,
          detailsContent: stringResources.benchInfoImageClassificationDesc,
        );
      case (BenchmarkId.imageClassificationOffline):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageClassificationOffline,
          detailsTitle: stringResources.benchInfoImageClassification,
          detailsContent: stringResources.benchInfoImageClassificationDesc,
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
      default:
        throw 'unhandled task id: ${task.id}';
    }
  }

  bool get isOffline => task.scenario == 'Offline';

  double get maxThroughput => task.maxThroughput;

  Widget get icon => _benchmarkIcons[task.id] ?? AppIcons.logo;

  Widget get iconWhite => _benchmarkIconsWhite[task.id] ?? AppIcons.logo;

  @override
  String toString() => 'Benchmark:${task.id}';
}

final _benchmarkIcons = {
  BenchmarkId.imageClassification: AppIcons.imageClassification,
  BenchmarkId.imageClassificationV2: AppIcons.imageClassification,
  BenchmarkId.objectDetection: AppIcons.objectDetection,
  BenchmarkId.imageSegmentationV2: AppIcons.imageSegmentation,
  BenchmarkId.naturalLanguageProcessing: AppIcons.languageProcessing,
  BenchmarkId.superResolution: AppIcons.superResolution,
  BenchmarkId.imageClassificationOffline: AppIcons.imageClassificationOffline,
  BenchmarkId.imageClassificationOfflineV2: AppIcons.imageClassificationOffline,
};

final _benchmarkIconsWhite = {
  BenchmarkId.imageClassification: AppIcons.imageClassificationWhite,
  BenchmarkId.imageClassificationV2: AppIcons.imageClassificationWhite,
  BenchmarkId.objectDetection: AppIcons.objectDetectionWhite,
  BenchmarkId.imageSegmentationV2: AppIcons.imageSegmentationWhite,
  BenchmarkId.naturalLanguageProcessing: AppIcons.languageProcessingWhite,
  BenchmarkId.superResolution: AppIcons.superResolutionWhite,
  BenchmarkId.imageClassificationOffline:
      AppIcons.imageClassificationOfflineWhite,
  BenchmarkId.imageClassificationOfflineV2:
      AppIcons.imageClassificationOfflineWhite,
};
