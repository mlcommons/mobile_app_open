import 'dart:convert';
import 'dart:io';

class LoadgenLogParser {
  static Future<Map<String, String>> parseData({
    required String filepath,
    required Set<String> requiredKeys,
  }) async {
    Map<String, String> result = {};
    await File(filepath)
        .openRead()
        .map(utf8.decode)
        .transform(const LineSplitter())
        .forEach((line) {
      const prefix = ':::MLLOG ';
      if (!line.startsWith(prefix)) {
        return;
      }
      line = line.substring(prefix.length);
      final json = jsonDecode(line);
      final lineKey = json['key'] as String;
      if (requiredKeys.contains(lineKey)) {
        result[lineKey] = json['value'].toString();
      }
    });

    return result;
  }
}
