import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'icons.dart';

class BenchmarkInfoItem {
  final String title;
  final String details;

  BenchmarkInfoItem({required this.title, required String details})
      : details = _trim(details);

  BenchmarkInfoItem.stub(String name)
      : title = name,
        details = name;

  static BenchmarkInfoItem? getBenchmarkInfoItem(
      String benchmarkCode, AppLocalizations stringResources) {
    switch (benchmarkCode) {
      case ('IC'):
        return BenchmarkInfoItem(
          title: stringResources.icInfo,
          details: stringResources.icInfoDescription,
        );
      case ('OD'):
        return BenchmarkInfoItem(
          title: stringResources.odInfo,
          details: stringResources.odInfoDescription,
        );
      case ('IS'):
        return BenchmarkInfoItem(
            title: stringResources.isInfo,
            details: stringResources.isInfoDescription);
      case ('LU'):
        return BenchmarkInfoItem(
            title: stringResources.luInfo,
            details: stringResources.luInfoDescription);
    }
  }
}

String getBenchmarkName(Benchmark benchmark, AppLocalizations stringResources) {
  switch (benchmark.info.code) {
    case ('IC'):
      if (benchmark.info.scenario == 'Offline') {
        return stringResources.imageClassificationOffline;
      }

      return stringResources.imageClassification;
    case ('OD'):
      return stringResources.objectDetection;
    case ('IS'):
      return stringResources.imageSegmentation;
    case ('LU'):
      return stringResources.languageProcessing;
  }

  return benchmark.id;
}

String _trim(String s) {
  return s
      .trim()
      .splitMapJoin(
        RegExp('\\n *'),
        onMatch: (_) => '\n',
      )
      .splitMapJoin(
        RegExp('\n\n?'),
        onMatch: (nl) => nl.end - nl.start == 1 ? ' ' : '\n',
      )
      .replaceAll('\n', '\n\n');
}

final BENCHMARK_ICONS = {
  'SingleStream': {
    'IC': Icons.image_classification,
    'OD': Icons.object_detection,
    'IS': Icons.image_segmentation,
    'LU': Icons.language_processing,
  },
  'Offline': {
    'IC': Icons.image_classification_offline,
  },
};

final BENCHMARK_ICONS_WHITE = {
  'SingleStream': {
    'IC': Icons.image_classification_white,
    'OD': Icons.object_detection_white,
    'IS': Icons.image_segmentation_white,
    'LU': Icons.language_processing_white,
  },
  'Offline': {
    'IC': Icons.image_classification_offline_white,
  },
};

final MAX_SCORE = {
  'IC_tpu_uint8': 508.0,
  'IC_tpu_float32': 508.0,
  'IC_tpu_uint8_offline': 508.0,
  'IC_tpu_float32_offline': 508.0,
  'OD_uint8': 288.0,
  'OD_float32': 288.0,
  'LU_int8': 12.0,
  'LU_float32': 12.0,
  'LU_gpu_float32': 12.0,
  'LU_nnapi_int8': 12.0,
  'IS_int8': 50.0,
  'IS_uint8': 50.0,
  'IS_float32': 50.0,
  'SUMMARY_MAX_SCORE': 155.0,
};
