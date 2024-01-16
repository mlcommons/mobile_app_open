import 'package:intl/intl.dart';

class ResultFileName {
  // Example fileName: 2023-06-06T13-38-01_125ef847-ca9a-45e0-bf36-8fd22f493b8d.json
  late String fileName;
  late String uuid;
  late DateTime dateTime;

  ResultFileName(this.uuid, this.dateTime) {
    final DateFormat formatter = DateFormat('yyyy-MM-ddTHH-mm-ss');
    final String datetimeString = formatter.format(dateTime);
    fileName = '${datetimeString}_$uuid.json';
  }

  ResultFileName.fromFileName(this.fileName) {
    var fileNameComponents = fileName.split('_');
    uuid = fileNameComponents.last.replaceAll('.json', '');
    final dateTimeString = fileNameComponents.first;
    final dateTimeStringComponents = dateTimeString.split('T');
    final dateString = dateTimeStringComponents.first;
    final timeString = dateTimeStringComponents.last.replaceAll('-', ':');
    dateTime = DateTime.parse('$dateString $timeString');
  }
}
