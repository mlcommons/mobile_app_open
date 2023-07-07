import 'package:firebase_storage/firebase_storage.dart';

class FirebaseStorageService {
  late final FirebaseStorage firebaseStorage;

  static const _uidPlaceholder = '<UID>';
  static const _filePlaceholder = '<FILE>';
  static const _pathTemplate = 'user/$_uidPlaceholder/result/$_filePlaceholder';

  upload(String jsonString, String uid, String fileName) async {
    final path = _getCloudStorageFilePath(uid, fileName);
    final storageRef = FirebaseStorage.instance.ref();
    final fileRef = storageRef.child(path);
    if (await _exists(fileRef)) {
      print('File $fileName already uploaded');
    } else {
      final metadata = SettableMetadata(contentType: 'text/plain');
      await fileRef.putString(jsonString, metadata: metadata);
      print('Uploaded to $path');
    }
  }

  Future<String> download(String uid, String fileName) async {
    final path = _getCloudStorageFilePath(uid, fileName);
    final url = firebaseStorage.ref(path).getDownloadURL();
    return url;
  }

  delete(String uid, String fileName) async {
    final path = _getCloudStorageFilePath(uid, fileName);
    await firebaseStorage.ref(path).delete();
  }

  Future<bool> _exists(Reference ref) async {
    try {
      await ref.getMetadata();
      return true;
    } catch (e) {
      return false;
    }
  }

  String _getCloudStorageFilePath(String uid, String fileName) {
    return _pathTemplate
        .replaceAll(_uidPlaceholder, uid)
        .replaceAll(_filePlaceholder, fileName);
  }
}
