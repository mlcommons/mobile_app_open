import 'dart:convert';
import 'dart:io' show HttpStatus;

import 'package:firebase_dart/firebase_dart.dart';
import 'package:http/http.dart' as http;

import 'package:mlperfbench_common/data/extended_result.dart';
import 'config.gen.dart';

class RestHelper {
  final UserCredential userInfo;

  RestHelper(this.userInfo);

  Future<void> upload(ExtendedResult result) async {
    const path = '/v0/upload';
    final jsonResult = result.toJson();
    if (userInfo.user == null) {
      throw 'Firebase Authentication issue';
    }
    var response =
        await http.post(Uri.parse('${FirebaseConfig.functionsUrl}$path'),
            headers: {
              'Authorization': '${await userInfo.user!.getIdToken()}',
            },
            body: JsonEncoder().convert(jsonResult));

    if (response.statusCode != HttpStatus.ok) {
      throw 'result uploading error: ${response.statusCode}: ${response.body}';
    }
  }
}
