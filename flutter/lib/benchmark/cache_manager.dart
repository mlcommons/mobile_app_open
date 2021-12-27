import 'dart:io';

class CacheManager {
  final String cacheDirectory;

  final HttpClient _httpClient = HttpClient();

  CacheManager(this.cacheDirectory);

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

  // If resource is cached, returns path to it
  // If resource doesn't exists and downloadMissing=true,
  //    downloads and returns path
  // If resource doesn't exists and downloadMissing=false,
  //    returns empty string
  Future<String> get(String url, bool downloadMissing) async {
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
}
