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
