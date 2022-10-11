import 'package:uuid/uuid.dart';

import 'package:mlperfbench_common/data/build_info/build_info.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/meta_info.dart';
import 'package:mlperfbench_common/data/results/backend_info.dart';
import 'package:mlperfbench_common/data/results/backend_settings.dart';
import 'package:mlperfbench_common/data/results/backend_settings_extra.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';
import 'package:mlperfbench_common/data/results/dataset_info.dart';

/// Generates sample data.
///
/// Generated data doesn't correspond to real values that could be obtained on some device.
/// It isn't even guaranteed to be logically consistent.
class SampleGenerator {
  BenchmarkRunResult get runResult => BenchmarkRunResult(
        throughput: 123.45,
        accuracy: Accuracy(
          normalized: 0.123,
          formatted: '12.3%',
        ),
        accuracy2: Accuracy(
          normalized: 0.123,
          formatted: '12.3%',
        ),
        dataset: DatasetInfo(
          name: 'Imagenet classification validation set',
          type: DatasetTypeEnum.imagenet,
          dataPath: 'app:///mlperf_datasets/imagenet/img',
          groundtruthPath:
              'app:///mlperf_datasets/imagenet/imagenet_val_full.txt',
        ),
        measuredDuration: 123.456,
        measuredSamples: 8,
        startDatetime: DateTime.now(),
        loadgenInfo: BenchmarkLoadgenInfo(
          duration: 10.6,
          validity: true,
        ),
      );
  BenchmarkExportResult get exportResult => BenchmarkExportResult(
        benchmarkId: 'id',
        benchmarkName: 'name',
        performanceRun: runResult,
        accuracyRun: runResult,
        backendInfo: BackendReportedInfo(
          filename: 'tflite',
          vendorName: 'tflite',
          backendName: 'libtflitebackend',
          acceleratorName: 'accelerator',
        ),
        minDuration: 10.5,
        minSamples: 8,
        backendSettings: BackendSettingsInfo(
          acceleratorCode: '',
          acceleratorDesc: '',
          configuration: '',
          modelPath: '',
          batchSize: 0,
          extraSettings: <BackendExtraSetting>[
            BackendExtraSetting(
              id: 'shards_num',
              name: 'Shards number',
              value: '2',
              valueName: '2',
            ),
          ],
        ),
        loadgenScenario:
            BenchmarkExportResult.parseLoadgenScenario('SingleStream'),
      );
  ExtendedResult get extendedResult => ExtendedResult(
        meta: ResultMetaInfo(
          uploadDate: DateTime.now(),
          uuid: const Uuid().v4(),
        ),
        buildInfo: BuildInfo(
          version: '1.0',
          buildNumber: '10qwe',
          gitBranch: 'feature',
          gitCommit: 'as91230jr90qwe',
          gitDirtyFlag: false,
          devTestFlag: true,
          backendList: <String>[
            'libtflitebackend',
          ],
          officialReleaseFlag: false,
        ),
        environmentInfo: EnvironmentInfo(
          manufacturer: 'Unknown',
          modelCode: 'Unknown PC',
          modelName: 'Unknown PC',
          osName: OsEnum.windows,
          osVersion: '10.0',
        ),
        results: [exportResult, exportResult],
      );
}