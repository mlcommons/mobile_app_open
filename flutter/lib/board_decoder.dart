import 'dart:convert';

import 'package:flutter/services.dart';

const boardDatabase = 'assets/android-boards/database.json';

class BoardDecoder {
  final Map<String, String> boards = {};

  BoardDecoder();

  Future<void> init() async {
    final data = await rootBundle.loadString(boardDatabase);
    final json = jsonDecode(data) as Map<String, dynamic>;
    for (var entry in json.entries) {
      final socInfo = entry.value as Map<String, dynamic>;
      boards[entry.key] = socInfo['SoC'] as String;
    }
  }
}
