import 'dart:typed_data';

import 'package:firebase_storage/firebase_storage.dart';

class FirebaseStorageService {
  late final FirebaseStorage firebaseStorage;

  static const _uidPlaceholder = '<UID>';
  static const _filePlaceholder = '<FILE>';
  static const _pathTemplate = 'user/$_uidPlaceholder/result/$_filePlaceholder';
  static const _oneMegabyte = 1024 * 1024;

  Future<void> upload(String jsonString, String uid, String fileName) async {
    final path = _getCloudStoragePath(uid, fileName);
    final fileRef = firebaseStorage.ref().child(path);
    if (await _exists(fileRef)) {
      print('File $fileName already uploaded');
    } else {
      final metadata = SettableMetadata(contentType: 'text/plain');
      await fileRef.putString(jsonString, metadata: metadata);
      print('Uploaded to $path');
    }
  }

  Future<String> download(String uid, String fileName) async {
    final path = _getCloudStoragePath(uid, fileName);
    final ref = firebaseStorage.ref(path);
    final Uint8List? data = await ref.getData(_oneMegabyte);
    if (data == null) {
      throw 'Cannot get data from path: $path';
    }
    final String content = String.fromCharCodes(data);
    return content;
  }

  Future<void> delete(String uid, String fileName) async {
    final path = _getCloudStoragePath(uid, fileName);
    await firebaseStorage.ref(path).delete();
  }

  Future<List<String>> list(String uid) async {
    final dir = _getCloudStoragePath(uid, '');
    final ListResult result = await firebaseStorage.ref(dir).listAll();
    final fileNames = result.items.map((e) => e.name).toList();
    return fileNames;
  }

  Future<bool> _exists(Reference ref) async {
    try {
      await ref.getMetadata();
      return true;
    } catch (e) {
      return false;
    }
  }

  String _getCloudStoragePath(String uid, String fileName) {
    return _pathTemplate
        .replaceAll(_uidPlaceholder, uid)
        .replaceAll(_filePlaceholder, fileName);
  }
}
