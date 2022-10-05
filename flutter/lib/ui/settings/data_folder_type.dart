import 'package:mlperfbench/app_constants.dart';

enum DataFolderType { default_, appFolder, custom }

DataFolderType parseDataFolderTypeSelection(String value) {
  if (value == DataFolderType.appFolder.name && defaultDataFolder.isNotEmpty) {
    // if defaultDataFolder is empty then default_ and appFolder value means the same,
    // so it doesn't make sense to have them as separate values
    return DataFolderType.appFolder;
  }
  if (value == DataFolderType.custom.name) {
    return DataFolderType.custom;
  }

  return DataFolderType.default_;
}
