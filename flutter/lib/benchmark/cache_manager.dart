import 'dart:io';

import 'package:archive/archive_io.dart';
import 'package:path_provider/path_provider.dart';
import 'package:uuid/uuid.dart';

class CacheManager {
  final cacheDirectory;

  final HttpClient _httpClient = HttpClient();
  late final String tmpDirectory;

  CacheManager(this.cacheDirectory);

  Future<void> init() async {
    tmpDirectory = (await getTemporaryDirectory()).path;
  }

  Future<File> downloadFile(String url) async {
    final result = File('$tmpDirectory/${Uuid().v4()}');
    await result.create(recursive: true);
    final succesStatusCode = 200;

    final response = await _httpClient
        .getUrl(Uri.parse(url))
        .then((request) => request.close());

    if (response.statusCode == succesStatusCode) {
      try {
        await response.pipe(result.openWrite());
      } catch (e) {
        throw 'Could not write to file ${result.path}';
      }
      return result;
    } else {
      throw 'Could not download file by url $url';
    }
  }

  Future<Directory> unzipFile(File zippedFile) async {
    final result = Directory('$tmpDirectory/${Uuid().v4()}');
    await result.create(recursive: true);

    try {
      final archive = ZipDecoder().decodeBytes(await zippedFile.readAsBytes());

      for (final archiveFile in archive) {
        final filePath = '${result.path}/${archiveFile.name}';
        final file = await File(filePath).create(recursive: true);

        final data = archiveFile.content as List<int>;
        await file.writeAsBytes(data);
      }
      await zippedFile.delete();

      return result;
    } catch (e) {
      await result.delete(recursive: true);
      throw 'Could not unzip file ${zippedFile.path}';
    }
  }
}
