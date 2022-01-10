import 'dart:io';

class FileCacheHelper {
  final String cacheDirectory;

  final HttpClient _httpClient = HttpClient();

  FileCacheHelper(this.cacheDirectory);

  String _sanitizePath(String path) {
    const illegalFilenameSymbols = '\\?%*:|"<>';
    return path.replaceAll(RegExp('[$illegalFilenameSymbols]'), '#');
  }

  String getResourceRelativePath(String url) {
    var result = url;
    var protocolIndex = result.indexOf('://');
    if (protocolIndex != -1) {
      result = result.substring(protocolIndex + '://'.length);
    }
    result = _sanitizePath(result);
    return result;
  }

  String getCachePath(String url) {
    return '$cacheDirectory/${getResourceRelativePath(url)}';
  }

  // If file is cached, returns path to it
  // If file is not cached and downloadMissing=true,
  //    downloads it and returns path to the file
  // If file is not cached and downloadMissing=false,
  //    returns empty string
  Future<String> get(String url, bool downloadMissing) async {
    var cachePath = getCachePath(url);
    if (await File(cachePath).exists()) {
      return cachePath;
    }
    if (!downloadMissing) {
      return '';
    }
    await _download(url);
    return cachePath;
  }

  Future<File> _download(String url) async {
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
