class ResultMetaInfo {
  static const String _tagUuid = 'uuid';
  static const String _tagUploadDate = 'upload_date';

  final String uuid;
  final String? uploadDate;

  ResultMetaInfo({required this.uuid, this.uploadDate});

  ResultMetaInfo.fromJson(Map<String, dynamic> json)
      : this(
            uuid: json[_tagUuid] as String,
            uploadDate: json[_tagUploadDate] == null
                ? null
                : json[_tagUploadDate] as String);

  Map<String, dynamic> toJson() => {
        _tagUuid: uuid,
        _tagUploadDate: uploadDate,
      };
}
