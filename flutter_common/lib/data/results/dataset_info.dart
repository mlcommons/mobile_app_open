import 'dataset_type.dart';

class DatasetInfo {
  static const String _tagName = 'name';
  static const String _tagType = 'type';
  static const String _tagDataPath = 'data_path';
  static const String _tagGroundtruthPath = 'groundtruth_path';

  final String name;
  final DatasetType type;
  final String dataPath;
  final String groundtruthPath;

  DatasetInfo(
      {required this.name,
      required this.type,
      required this.dataPath,
      required this.groundtruthPath});

  DatasetInfo.fromJson(Map<String, dynamic> json)
      : this(
            name: json[_tagName] as String,
            type: DatasetType.fromJson(json[_tagType] as String),
            dataPath: json[_tagDataPath] as String,
            groundtruthPath: json[_tagGroundtruthPath] as String);

  Map<String, dynamic> toJson() => {
        _tagName: name,
        _tagType: type,
        _tagDataPath: dataPath,
        _tagGroundtruthPath: groundtruthPath,
      };
}
