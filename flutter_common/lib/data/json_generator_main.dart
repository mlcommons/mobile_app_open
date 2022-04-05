import 'dart:convert';
import 'dart:io';

import 'package:uuid/uuid.dart';

import 'environment_info.dart';
import 'export_result.dart';
import 'extended_result.dart';

//
// This file is intended to be used
// to automatically generate json schema
// for firebase functions.
//

const fileNameEnv = 'jsonFileName';

Future<void> main() async {
  var exportResult = ExportResult(
    id: 'id',
    accuracy: 'N/A',
    throughput: '123.45',
    backendName: 'backend',
    acceleratorName: 'accelerator',
    minDuration: '10',
    duration: '123.456',
    minSamples: '8',
    numSamples: '8',
    mode: 'performance_lite_mode',
    datetime: 'date will be here',
    batchSize: 0,
    shardsNum: 0,
  );
  var extendedResult = ExtendedResult(
    uuid: Uuid().v4(),
    uploadDate: 'upload date will be here',
    envInfo: EnvironmentInfo(
      appVersion: '1.0',
      manufacturer: 'unknown',
      model: 'unknown',
      os: 'os',
      osVersion: '10.0',
    ),
    results: ExportResultList([exportResult, exportResult]),
  );
  if (!const bool.hasEnvironment(fileNameEnv)) {
    print('pass --define=$fileNameEnv=<value> to specify file to write');
    exit(1);
  }
  const filename = String.fromEnvironment(fileNameEnv);
  print('writing results to $filename');
  final file = File(filename);
  await file.create();
  await file.writeAsString(JsonEncoder().convert(extendedResult));
}
