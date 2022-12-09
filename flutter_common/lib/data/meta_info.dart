import 'package:json_annotation/json_annotation.dart';

part 'meta_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class ResultMetaInfo {
  final String uuid;
  final DateTime creationDate;
  final DateTime? uploadDate;

  ResultMetaInfo({
    required this.uuid,
    required this.creationDate,
    DateTime? uploadDate,
  }) : uploadDate = uploadDate?.toUtc();

  factory ResultMetaInfo.fromJson(Map<String, dynamic> json) =>
      _$ResultMetaInfoFromJson(json);

  Map<String, dynamic> toJson() => _$ResultMetaInfoToJson(this);
}
