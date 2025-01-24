import 'dart:convert';
import 'dart:ui';

import 'package:mlperfbench/resources/resource.dart';

bool isInternetResource(String uri) =>
    uri.startsWith('http://') || uri.startsWith('https://');

const String _assetPrefix = 'asset://';
bool isAsset(String uri) => uri.startsWith(_assetPrefix);
String stripAssetPrefix(String uri) => uri.substring(_assetPrefix.length);

List<String> filterInternetResources(List<Resource> resources) {
  final result = <String>[];
  for (var r in resources) {
    if (isInternetResource(r.path)) {
      result.add(r.path);
    }
  }
  return result;
}

String jsonToStringIndented(dynamic json) {
  const jsonEncoder = JsonEncoder.withIndent('  ');
  return jsonEncoder.convert(json);
}

/// Lineraly interpolates between [start] and [end] using a [factor] between [valueStart] and [ValueEnd] instead of between 0 and 1.
double? lerpRange(
    num? start, num? end, double valueStart, double valueEnd, double factor) {
  final double valueEndNormalized = valueEnd - valueStart;
  final double factorNormalized = (factor - valueStart) / valueEndNormalized;

  return lerpDouble(start, end, factorNormalized);
}
