import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/data/result_file_name.dart';

void main() {
  group('ResultFileName', () {
    test('Init from file name', () {
      const fileName =
          '2023-06-06T13-38-01_125ef847-ca9a-45e0-bf36-8fd22f493b8d.json';
      const uuid = '125ef847-ca9a-45e0-bf36-8fd22f493b8d';
      final dateTime = DateTime.parse('2023-06-06T13:38:01');
      final resultFileName = ResultFileName.fromFileName(fileName);
      expect(resultFileName.fileName, equals(fileName));
      expect(resultFileName.uuid, equals(uuid));
      expect(resultFileName.dateTime, equals(dateTime));
    });

    test('Init from UUID and date time', () {
      const fileName =
          '2023-06-06T13-38-01_125ef847-ca9a-45e0-bf36-8fd22f493b8d.json';
      const uuid = '125ef847-ca9a-45e0-bf36-8fd22f493b8d';
      final dateTime = DateTime.parse('2023-06-06T13:38:01');
      final resultFileName = ResultFileName(uuid, dateTime);
      expect(resultFileName.fileName, equals(fileName));
      expect(resultFileName.uuid, equals(uuid));
      expect(resultFileName.dateTime, equals(dateTime));
    });
  });
}
