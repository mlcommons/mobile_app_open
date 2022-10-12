import 'package:json_annotation/json_annotation.dart';

part 'dataset_info.g.dart';

enum DatasetTypeEnum {
  @JsonValue('IMAGENET')
  imagenet,
  @JsonValue('COCO')
  coco,
  @JsonValue('ADE20K')
  ade20k,
  @JsonValue('SQUAD')
  squad,
}

extension DatasetTypeExtension on DatasetTypeEnum {
  String get humanName {
    return _$DatasetTypeEnumEnumMap[this]!;
  }
}

@JsonSerializable(fieldRename: FieldRename.snake)
class DatasetInfo {
  final String name;
  final DatasetTypeEnum type;
  final String dataPath;
  final String groundtruthPath;

  DatasetInfo({
    required this.name,
    required this.type,
    required this.dataPath,
    required this.groundtruthPath,
  });

  factory DatasetInfo.fromJson(Map<String, dynamic> json) =>
      _$DatasetInfoFromJson(json);

  Map<String, dynamic> toJson() => _$DatasetInfoToJson(this);

  static DatasetTypeEnum parseDatasetType(String value) {
    return _$DatasetTypeEnumEnumMap.entries
        .firstWhere((element) => element.value == value)
        .key;
  }
}
