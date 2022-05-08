import 'dart:convert';

bool isInternetResource(String uri) =>
    uri.startsWith('http://') || uri.startsWith('https://');

List<String> filterInternetResources(List<String> resources) {
  final result = <String>[];
  for (var r in resources) {
    if (isInternetResource(r)) {
      result.add(r);
    }
  }
  return result;
}

String base64ToHex(String source) =>
    base64Decode(LineSplitter.split(source).join())
        .map((e) => e.toRadixString(16).padLeft(2, '0'))
        .join();
