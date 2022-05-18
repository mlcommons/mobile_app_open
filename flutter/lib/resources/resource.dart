enum ResourceTypeEnum { model, datasetData, datasetGroundtruth }

class Resource {
  final String path;
  final ResourceTypeEnum type;
  final String? md5Checksum;

  Resource({
    required this.path,
    required this.type,
    this.md5Checksum,
  });
}
