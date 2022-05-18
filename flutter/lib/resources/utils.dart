import 'package:mlperfbench/resources/resource.dart';

bool isInternetResource(String uri) =>
    uri.startsWith('http://') || uri.startsWith('https://');

List<String> filterInternetResources(List<Resource> resources) {
  final result = <String>[];
  for (var r in resources) {
    if (isInternetResource(r.path)) {
      result.add(r.path);
    }
  }
  return result;
}
