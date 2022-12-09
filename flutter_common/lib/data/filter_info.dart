import 'package:json_annotation/json_annotation.dart';

part 'filter_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class FilterInfo {
  final String backendName;

  FilterInfo({
    required this.backendName,
  });

  factory FilterInfo.fromJson(Map<String, dynamic> json) =>
      _$FilterInfoFromJson(json);

  Map<String, dynamic> toJson() => _$FilterInfoToJson(this);
}
