class SelectedBackendInfo {
  static const String _tagFilename = 'filename';
  static const String _tagVendorName = 'vendor_name';
  static const String _tagBackendName = 'backend_name';

  final String filename;
  final String vendor;
  final String name;

  SelectedBackendInfo(
      {required this.filename,
      required this.vendor,
      required this.name});

  SelectedBackendInfo.fromJson(Map<String, dynamic> json)
      : this(
            filename: json[_tagFilename] as String,
            vendor: json[_tagVendorName] as String,
            name: json[_tagBackendName] as String);

  Map<String, dynamic> toJson() => {
        _tagFilename: filename,
        _tagVendorName: vendor,
        _tagBackendName: name,
      };
}
