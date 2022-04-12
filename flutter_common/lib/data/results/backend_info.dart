class BackendReportedInfo {
  static const String _tagFilename = 'filename';
  static const String _tagVendorName = 'vendor_name';
  static const String _tagBackendName = 'backend_name';
  static const String _tagAcceleratorName = 'accelerator_name';

  final String filename;
  final String vendor;
  final String name;
  final String accelerator;

  BackendReportedInfo(
      {required this.filename,
      required this.vendor,
      required this.name,
      required this.accelerator,
      });

  BackendReportedInfo.fromJson(Map<String, dynamic> json)
      : this(
            filename: json[_tagFilename] as String,
            vendor: json[_tagVendorName] as String,
            name: json[_tagBackendName] as String,
            accelerator: json[_tagAcceleratorName] as String,
            );

  Map<String, dynamic> toJson() => {
        _tagFilename: filename,
        _tagVendorName: vendor,
        _tagBackendName: name,
        _tagAcceleratorName: accelerator,
      };
}
