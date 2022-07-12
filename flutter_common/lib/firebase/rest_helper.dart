import 'dart:convert';
import 'dart:io' show HttpStatus;

import 'package:firebase_dart/firebase_dart.dart';
import 'package:http/http.dart' as http;

import 'package:mlperfbench_common/data/extended_result.dart';
import 'config.gen.dart';

class RestHelper {
  final UserCredential userInfo;

  static final uploadUri = getUrl('upload');
  static final fetchFirstUri = getUrl('fetch/first');
  static final fetchNextUri = getUrl('fetch/next');
  static final fetchIdUri = getUrl('fetch/id');

  RestHelper(this.userInfo);

  Future<String> getAuthToken() async {
    return await userInfo.user!.getIdToken();
  }

  static Uri getUrl(String path) {
    return Uri.parse(
        '${FirebaseConfig.functionsUrl}/${FirebaseConfig.functionsPrefix}/$path');
  }

  Future<void> upload(ExtendedResult result) async {
    final jsonResult = result.toJson();
    if (userInfo.user == null) {
      throw 'Firebase Authentication issue';
    }
    var response = await http.post(uploadUri,
        headers: {
          'Authorization': await getAuthToken(),
        },
        body: const JsonEncoder().convert(jsonResult));

    if (response.statusCode != HttpStatus.created) {
      throw 'error ${response.statusCode}: ${response.body}';
    }
  }

  Future<List<ExtendedResult>> fetchFirst({int pageSize = 20}) async {
    final token = await getAuthToken();
    var response = await http.get(fetchFirstUri, headers: {
      'Authorization': token,
      'page-size': pageSize.toString(),
    });
    if (response.statusCode != HttpStatus.ok) {
      throw 'error ${response.statusCode}: ${response.body}';
    }
    print(response.body);
    final result = <ExtendedResult>[];
    for (var item in jsonDecode(response.body) as List<dynamic>) {
      final decoded = ExtendedResult.fromJson(item);
      result.add(decoded);
    }
    return result;
  }

  Future<List<ExtendedResult>> fetchNext(
      {int pageSize = 20, required String uuidCursor}) async {
    final token = await getAuthToken();
    var response = await http.get(fetchNextUri, headers: {
      'Authorization': token,
      'page-size': pageSize.toString(),
      'uuid-cursor': uuidCursor,
    });
    if (response.statusCode != HttpStatus.ok) {
      throw 'error ${response.statusCode}: ${response.body}';
    }
    print(response.body);
    final result = <ExtendedResult>[];
    for (var item in jsonDecode(response.body) as List<dynamic>) {
      final decoded = ExtendedResult.fromJson(item);
      result.add(decoded);
    }
    return result;
  }

  Future<ExtendedResult> fetchId({required String uuid}) async {
    final token = await getAuthToken();
    var response = await http.get(fetchIdUri, headers: {
      'Authorization': token,
      'uuid': uuid,
    });
    if (response.statusCode != HttpStatus.ok) {
      throw 'error ${response.statusCode}: ${response.body}';
    }
    print(response.body);
    return ExtendedResult.fromJson(jsonDecode(response.body));
  }
}
