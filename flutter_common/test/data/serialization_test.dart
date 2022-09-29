import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench_common/data/results/backend_info.dart';

void main() {
  group('json serialization', () {
    final example = BackendReportedInfo(
      acceleratorName: '1',
      backendName: '2',
      filename: '3',
      vendorName: '4',
    );
    test('add key', () async {
      final exampleJson = example.toJson();
      exampleJson['some_key'] = 'value';

      final parsed = BackendReportedInfo.fromJson(exampleJson);
      expect(parsed.acceleratorName, example.acceleratorName);
      expect(parsed.backendName, example.backendName);
      expect(parsed.filename, example.filename);
      expect(parsed.vendorName, example.vendorName);
    });
    test('remove key', () async {
      final exampleJson = example.toJson();
      exampleJson.remove('filename');

      expect(() {
        BackendReportedInfo.fromJson(exampleJson);
      }, throwsA(isA<TypeError>()));
    });
    test('rename key', () async {
      final exampleJson = example.toJson();
      exampleJson['new_name'] = exampleJson['filename'];
      exampleJson.remove('filename');

      expect(() {
        BackendReportedInfo.fromJson(exampleJson);
      }, throwsA(isA<TypeError>()));
    });
  });
}
