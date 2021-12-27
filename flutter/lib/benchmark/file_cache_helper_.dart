import 'dart:io';

class FileCacheHelper {
  final String cacheDirectory;

  final HttpClient _httpClient = HttpClient();

  FileCacheHelper(this.cacheDirectory);

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

  // If resource is cached, returns path to it
  // If resource doesn't exists and downloadMissing=true,
  //    downloads and returns path
  // If resource doesn't exists and downloadMissing=false,
  //    returns empty string
  Future<String> get(String url, bool downloadMissing) async {
    var cachePath = getCachePath(url);
    if (await File(cachePath).exists()) {
      return cachePath;
    }
    if (!downloadMissing) {
      return '';
    }
    await download(url);
    return cachePath;
  }

  Future<File> download(String url) async {
    const succesStatusCode = 200;

    final response = await _httpClient
        .getUrl(Uri.parse(url))
        .then((request) => request.close());

    if (response.statusCode != succesStatusCode) {
      throw 'Could not download file by url: status ${response.statusCode}, url: $url';
    }

    final result = File(getCachePath(url));
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
