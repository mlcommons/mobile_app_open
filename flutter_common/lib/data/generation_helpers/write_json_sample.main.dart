import 'dart:convert';
import 'dart:io';

import 'package:mlperfbench_common/data/extended_result.dart';
import 'package:mlperfbench_common/data/generation_helpers/sample_generator.dart';

//
// This file generates a sample of result data and writes it into specified place.
// It is intended to be used
// to automatically generate json schema
// for firebase functions.
//

const fileNameEnv = 'jsonFileName';

Future<void> main() async {
  if (!const bool.hasEnvironment(fileNameEnv)) {
    print('pass --define=$fileNameEnv=<value> to specify file to write');
    exit(1);
  }
  const filename = String.fromEnvironment(fileNameEnv);
  print('writing results to $filename');
  final file = File(filename);
  await file.create();
  final generator = SampleGenerator();
  final sample = generator.extendedResult;
  final converted = const JsonEncoder().convert(sample);
  // at least check that parsing doesn't throw exceptions
  final _ = ExtendedResult.fromJson(jsonDecode(converted));
  await file.writeAsString(converted);
}
