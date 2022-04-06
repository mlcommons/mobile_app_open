import 'backend_settings_extra.dart';

class BackendSettingsInfo {
  static const String _tagAcceleratorCode = 'accelerator_code';
  static const String _tagAcceleratorDesc = 'accelerator_desc';
  static const String _tagConfiguration = 'configuration';
  static const String _tagModelPath = 'model_path';
  static const String _tagExtraSettings = 'extra_settings';

  final String acceleratorCode;
  final String acceleratorDesc;
  final String configuration;
  final String modelPath;
  final BackendExtraSettingList extraSettings;

  BackendSettingsInfo(
      {required this.acceleratorCode,
      required this.acceleratorDesc,
      required this.configuration,
      required this.modelPath,
      required this.extraSettings});

  BackendSettingsInfo.fromJson(Map<String, dynamic> json)
      : this(
            acceleratorCode: json[_tagAcceleratorCode] as String,
            acceleratorDesc: json[_tagAcceleratorDesc] as String,
            configuration: json[_tagConfiguration] as String,
            modelPath: json[_tagModelPath] as String,
            extraSettings: BackendExtraSettingList.fromJson(
                json[_tagExtraSettings] as List<dynamic>));

  Map<String, dynamic> toJson() => {
        _tagAcceleratorCode: acceleratorCode,
        _tagAcceleratorDesc: acceleratorDesc,
        _tagConfiguration: configuration,
        _tagModelPath: modelPath,
        _tagExtraSettings: extraSettings,
      };
}
