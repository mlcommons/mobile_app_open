class ResultMetaInfo {
  static const String _tagUuid = 'uuid';
  static const String _tagModel = 'upload_date';

  final String uuid;
  final String uploadDate;

  ResultMetaInfo({required this.uuid, required this.uploadDate});

  ResultMetaInfo.fromJson(Map<String, dynamic> json)
      : this(
            uuid: json[_tagUuid] as String,
            uploadDate: json[_tagModel] as String);

  Map<String, dynamic> toJson() => {
        _tagUuid: uuid,
        _tagModel: uploadDate,
      };
}
