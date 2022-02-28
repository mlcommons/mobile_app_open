import 'dart:convert';
import 'dart:io' show File, HttpStatus;

import 'package:firebase_dart/firebase_dart.dart';
import 'package:http/http.dart' as http;

import 'package:mlperfbench/data/extended_result.dart';
import 'config.gen.dart';

class RestHelper {
  final UserCredential userInfo;

  RestHelper(this.userInfo);

  Future<void> upload(ExtendedResult result) async {
    final jsonResult = result.toJson();
    var response =
        await http.post(Uri.parse('http://$firebaseFunctionsUrl/upload'),
            headers: {
              'Content-Type': 'application/json',
              'Authorization': '${await userInfo.user!.getIdToken()}',
            },
            body: JsonEncoder().convert(jsonResult));

    // await File('Z:/test.json').writeAsString(JsonEncoder().convert(jsonResult));

    if (response.statusCode != HttpStatus.ok) {
      throw 'result uploading error: ${response.statusCode}';
    }
  }
}
