enum ResourceTypeEnum { model, datasetData, datasetGroundtruth }

class Resource {
  final ResourceTypeEnum type;
  final String path;
  final String md5Checksum;

  Resource({
    required this.type,
    required this.path,
    required this.md5Checksum,
  });

  @override
  int get hashCode => Object.hash(path, type, md5Checksum);

  @override
  bool operator ==(Object other) =>
      other is Resource &&
      other.runtimeType == runtimeType &&
      other.path == path &&
      other.type == type &&
      other.md5Checksum == md5Checksum;

  @override
  String toString() =>
      'Resource(path:$path,type:$type,md5Checksum:$md5Checksum)';
}
