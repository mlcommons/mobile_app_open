import 'package:flutter/material.dart';

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
      case ('image_classification'):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageClassification,
          detailsTitle: stringResources.benchInfoImageClassification,
          detailsContent: stringResources.benchInfoImageClassificationDesc,
        );
      case ('image_classification_offline'):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageClassificationOffline,
          detailsTitle: stringResources.benchInfoImageClassification,
          detailsContent: stringResources.benchInfoImageClassificationDesc,
        );
      case ('object_detection'):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameObjectDetection,
          detailsTitle: stringResources.benchInfoObjectDetection,
          detailsContent: stringResources.benchInfoObjectDetectionDesc,
        );
      case ('image_segmentation_v2'):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameImageSegmentation,
          detailsTitle: stringResources.benchInfoImageSegmentation,
          detailsContent: stringResources.benchInfoImageSegmentationDesc,
        );
      case ('natural_language_processing'):
        return BenchmarkLocalizationInfo(
          name: stringResources.benchNameLanguageProcessing,
          detailsTitle: stringResources.benchInfoLanguageProcessing,
          detailsContent: stringResources.benchInfoLanguageProcessingDesc,
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
  'image_classification': AppIcons.imageClassification,
  'object_detection': AppIcons.objectDetection,
  'image_segmentation_v2': AppIcons.imageSegmentation,
  'natural_language_processing': AppIcons.languageProcessing,
  'image_classification_offline': AppIcons.imageClassificationOffline,
};

final _benchmarkIconsWhite = {
  'image_classification': AppIcons.imageClassificationWhite,
  'object_detection': AppIcons.objectDetectionWhite,
  'image_segmentation_v2': AppIcons.imageSegmentationWhite,
  'natural_language_processing': AppIcons.languageProcessingWhite,
  'image_classification_offline': AppIcons.imageClassificationOfflineWhite,
};
