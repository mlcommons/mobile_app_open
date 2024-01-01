import 'package:json_annotation/json_annotation.dart';

part 'backend_info.g.dart';

@JsonSerializable(fieldRename: FieldRename.snake)
class BackendReportedInfo {
  final String filename;
  final String vendorName;
  final String backendName;
  final String acceleratorName;

  BackendReportedInfo({
    required this.filename,
    required this.vendorName,
    required this.backendName,
    required this.acceleratorName,
  });

  factory BackendReportedInfo.fromJson(Map<String, dynamic> json) =>
      _$BackendReportedInfoFromJson(json);

  Map<String, dynamic> toJson() => _$BackendReportedInfoToJson(this);
}
