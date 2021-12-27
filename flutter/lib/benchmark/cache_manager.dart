import 'dart:io';

import 'package:archive/archive_io.dart';
import 'package:path_provider/path_provider.dart';
import 'package:uuid/uuid.dart';

class CacheManager {
  static const _zipPattern = '.zip';
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

  String getCachePath(String url) {
    return '$cacheDirectory/${getResourceRelativePath(url)}';
  }

  Future<bool> isFileCached(String url) async {
    // TODO pass cachedPath?
    var cachedPath = getCachePath(url);
    var file = File(cachedPath);
    return await file.exists();
  }

  bool isResourceAnArchive(String cachedPath) {
    return cachedPath.endsWith(_zipPattern);
  }

  String getArchiveFolder(String cachedPath) {
    return cachedPath.substring(0, cachedPath.length - _zipPattern.length);
  }

  Future<bool> isResourceCached(String url) async {
    var cachedPath = getCachePath(url);
    if (isResourceAnArchive(cachedPath)) {
      return Directory(getArchiveFolder(cachedPath)).exists();
    }
    return isFileCached(url);
  }

  // If resource is cached, returns path to it
  // If resource doesn't exists and downloadMissing=true,
  //    downloads and returns path
  // If resource doesn't exists and downloadMissing=false,
  //    returns empty string
  Future<String> getFile(String url, bool downloadMissing) async {
    var cachePath = getCachePath(url);
    if (await isFileCached(url)) {
      return cachePath;
    }
    if (!downloadMissing) {
      return '';
    }
    await downloadFile(url);
    return cachePath;
  }

  // If resource is cached, returns path to it
  // If resource doesn't exists and downloadMissing=true,
  //    downloads and returns path
  // If resource doesn't exists and downloadMissing=false,
  //    returns empty string
  Future<String> getArchive(String url, bool downloadMissing) async {
    var cachePath = getArchiveFolder(getCachePath(url));
    if (await Directory(cachePath).exists()) {
      return cachePath;
    }
    if (!downloadMissing) {
      return '';
    }
    var file = await downloadFile(url);
    final unZippedDirectory = await _unzipFile(file);
    await file.delete();
    return unZippedDirectory.path;
  }

  Future<File> downloadFile(String url) async {
    final succesStatusCode = 200;

    final response = await _httpClient
        .getUrl(Uri.parse(url))
        .then((request) => request.close());

    if (response.statusCode != succesStatusCode) {
      throw 'Could not download file by url: status ${response.statusCode}, url: $url';
    }

    final result = File('${getCachePath(url)}');
    await result.create(recursive: true);
    try {
      await response.pipe(result.openWrite());
    } catch (e) {
      await result.delete();
      throw 'Could not write to file ${result.path}: $e';
    }
    return result;
  }

  Future<Directory> _unzipFile(File zippedFile) async {
    final result = Directory(getArchiveFolder(zippedFile.path));
    await result.create(recursive: true);

    try {
      final archive = ZipDecoder().decodeBytes(await zippedFile.readAsBytes());

      for (final archiveFile in archive) {
        final filePath = '${result.path}/${archiveFile.name}';
        final file = await File(filePath).create(recursive: true);

        final data = archiveFile.content as List<int>;
        await file.writeAsBytes(data);
      }

      return result;
    } catch (e) {
      await result.delete(recursive: true);
      throw 'Could not unzip file ${zippedFile.path}: $e';
    }
  }
}
