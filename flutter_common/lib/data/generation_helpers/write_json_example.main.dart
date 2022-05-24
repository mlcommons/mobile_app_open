import 'dart:convert';
import 'dart:io';

import 'package:uuid/uuid.dart';

import '../build_info/backend_list.dart';
import '../build_info/build_info.dart';
import '../environment/environment_info.dart';
import '../environment/os_enum.dart';
import '../extended_result.dart';
import '../meta_info.dart';
import '../results/backend_info.dart';
import '../results/backend_settings.dart';
import '../results/backend_settings_extra.dart';
import '../results/benchmark_result.dart';
import '../results/dataset_info.dart';
import '../results/dataset_type.dart';
import '../results/loadgen_scenario.dart';

//
// This file is intended to be used
// to automatically generate json schema
// for firebase functions.
//

const fileNameEnv = 'jsonFileName';

Future<void> main() async {
  final runResult = BenchmarkRunResult(
    throughput: 123.45,
    accuracy: 123.45,
    datasetInfo: DatasetInfo(
      name: 'Imagenet classification validation set',
      type: DatasetType.fromJson('IMAGENET'),
      dataPath: 'app:///mlperf_datasets/imagenet/img',
      groundtruthPath: 'app:///mlperf_datasets/imagenet/imagenet_val_full.txt',
    ),
    measuredDurationMs: 123.456,
    measuredSamples: 8,
    startDatetime: DateTime.now(),
    loadgenValidity: true,
  );
  var exportResult = BenchmarkExportResult(
    benchmarkId: 'id',
    benchmarkName: 'name',
    performance: runResult,
    accuracy: runResult,
    backendInfo: BackendReportedInfo(
      filename: 'tflite',
      vendor: 'tflite',
      name: 'libtflitebackend',
      accelerator: 'accelerator',
    ),
    minDurationMs: 10.5,
    minSamples: 8,
    backendSettingsInfo: BackendSettingsInfo(
      acceleratorCode: '',
      acceleratorDesc: '',
      configuration: '',
      modelPath: '',
      batchSize: 0,
      extraSettings: BackendExtraSettingList(
        <BackendExtraSetting>[
          BackendExtraSetting(
            id: 'shards_num',
            name: 'Shards number',
            value: '2',
            valueName: '2',
          ),
        ],
      ),
    ),
    loadgenScenario: LoadgenScenario.fromJson('SingleStream'),
  );
  var extendedResult = ExtendedResult(
    meta: ResultMetaInfo(
      uploadDate: DateTime.now(),
      uuid: Uuid().v4(),
    ),
    buildInfo: BuildInfo(
      version: '1.0',
      buildNumber: '10qwe',
      gitBranch: 'feature',
      gitCommit: 'as91230jr90qwe',
      gitDirtyFlag: false,
      devTestFlag: true,
      backends: BackendList(<String>[
        'libtflitebackend',
      ]),
      officialReleaseFlag: false,
    ),
    envInfo: EnvironmentInfo(
      manufacturer: 'unknown',
      model: 'unknown',
      osName: OsName.fromJson('windows'),
      osVersion: '10.0',
    ),
    results: BenchmarkExportResultList([exportResult, exportResult]),
  );
  if (!const bool.hasEnvironment(fileNameEnv)) {
    print('pass --define=$fileNameEnv=<value> to specify file to write');
    exit(1);
  }
  const filename = String.fromEnvironment(fileNameEnv);
  print('writing results to $filename');
  final file = File(filename);
  await file.create();
  final converted = JsonEncoder().convert(extendedResult);
  // at least check that parsing doesn't throw exceptions
  final _ = ExtendedResult.fromJson(jsonDecode(converted));
  await file.writeAsString(converted);
}
