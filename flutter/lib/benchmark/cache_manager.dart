import 'dart:io';

import 'package:archive/archive_io.dart';
import 'package:path_provider/path_provider.dart';
import 'package:uuid/uuid.dart';

class CacheManager {
  final String cacheDirectory;

  final HttpClient _httpClient = HttpClient();
  late final String tmpDirectory;

  CacheManager(this.cacheDirectory);

  Future<void> init() async {
    tmpDirectory = (await getTemporaryDirectory()).path;
  }

  String sanitazePath(String path) {
    const illegalFilenameSymbols = '\\?%*:|"<>';
    return path.replaceAll(RegExp('[$illegalFilenameSymbols]'), '#');
  }

  String getResourceRelativePath(String url) {
    var result = url;
    var protocolIndex = result.indexOf('://');
    if (protocolIndex != -1) {
      result = result.substring(protocolIndex + '://'.length);
    }
    result = sanitazePath(result);
    return result;
  }

  String getCachePath(String resourceUrl) {
    return '$cacheDirectory/${getResourceRelativePath(resourceUrl)}';
  }

  Future<File> downloadFile(String url) async {
    final succesStatusCode = 200;

    final response = await _httpClient
        .getUrl(Uri.parse(url))
        .then((request) => request.close());

    if (response.statusCode != succesStatusCode) {
      throw 'Could not download file by url: status ${response.statusCode}, url: $url';
    }

    final result = File('$tmpDirectory/${Uuid().v4()}');
    await result.create(recursive: true);
    try {
      await response.pipe(result.openWrite());
    } catch (e) {
      await result.delete();
      throw 'Could not write to file ${result.path}: $e';
    }
    return result;
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
