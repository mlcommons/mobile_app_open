enum DatasetTypeEnum { imagenet, coco, ade20k, squad }

class DatasetType {
  static const String _serializedImagenet = 'IMAGENET';
  static const String _serializedCoco = 'COCO';
  static const String _serializedAde20k = 'ADE20K';
  static const String _serializedSquad = 'SQUAD';

  final DatasetTypeEnum value;

  DatasetType(this.value);

  static DatasetType fromJson(String serialized) {
    switch (serialized) {
      case _serializedImagenet:
        return DatasetType(DatasetTypeEnum.imagenet);
      case _serializedCoco:
        return DatasetType(DatasetTypeEnum.coco);
      case _serializedAde20k:
        return DatasetType(DatasetTypeEnum.ade20k);
      case _serializedSquad:
        return DatasetType(DatasetTypeEnum.squad);
      default:
        throw 'invalid DatasetType value: $serialized';
    }
  }

  String toJson() {
    switch (value) {
      case DatasetTypeEnum.imagenet:
        return _serializedImagenet;
      case DatasetTypeEnum.coco:
        return _serializedCoco;
      case DatasetTypeEnum.ade20k:
        return _serializedAde20k;
      case DatasetTypeEnum.squad:
        return _serializedSquad;
      default:
        throw 'invalid DatasetType value: $value';
    }
  }
}
