class ExportResult {
  static const String _tagId = 'id';
  static const String _tagConfiguration = 'configuration';
  static const String _tagRuntime = 'runtime';
  static const String _tagThroughput = 'throughput';
  static const String _tagAccuracy = 'accuracy';
  static const String _tagMinDuration = 'min_duration';
  static const String _tagDuration = 'duration';
  static const String _tagMinSamples = 'min_samples';
  static const String _tagNumSamples = 'num_samples';
  static const String _tagShardsNum = 'shards_num';
  static const String _tagBatchSize = 'batch_size';
  static const String _tagMode = 'mode';
  static const String _tagDatetime = 'datetime';
  static const String _tagBackendName = 'backendName';
  static const String _tagAcceleratorName = 'acceleratorName';

  final String id;
  final String throughput;
  final String accuracy;
  // numeric values are saved as string to match old android app behavior
  final String minDuration;
  final String duration;
  final String minSamples;
  final String numSamples;
  final int shardsNum;
  final int batchSize;
  final String mode;
  final String datetime;
  final String backendName;
  final String acceleratorName;

  ExportResult(
      {required this.id,
      required this.throughput,
      required this.accuracy,
      required this.minDuration,
      required this.duration,
      required this.minSamples,
      required this.numSamples,
      required this.shardsNum,
      required this.batchSize,
      required this.mode,
      required this.datetime,
      required this.backendName,
      required this.acceleratorName});

  ExportResult.fromJson(Map<String, dynamic> json)
      : this(
            id: json[_tagId] as String,
            throughput: json[_tagThroughput] as String,
            accuracy: json[_tagAccuracy] as String,
            minDuration: json[_tagMinDuration] as String,
            duration: json[_tagDuration] as String,
            minSamples: json[_tagMinSamples] as String,
            numSamples: json[_tagNumSamples] as String,
            shardsNum: json[_tagShardsNum] as int,
            batchSize: json[_tagBatchSize] as int,
            mode: json[_tagMode] as String,
            datetime: json[_tagDatetime] as String,
            backendName: json[_tagBackendName] as String,
            acceleratorName: json[_tagAcceleratorName] as String);

  Map<String, dynamic> toJson() => {
        _tagId: id,
        _tagConfiguration: {
          _tagRuntime: '',
        },
        _tagThroughput: throughput,
        _tagAccuracy: accuracy,
        _tagMinDuration: minDuration,
        _tagDuration: duration,
        _tagMinSamples: minSamples,
        _tagNumSamples: numSamples,
        _tagShardsNum: shardsNum,
        _tagBatchSize: batchSize,
        _tagMode: mode,
        _tagDatetime: datetime,
        _tagBackendName: backendName,
        _tagAcceleratorName: acceleratorName,
      };
}

class ExportResultList {
  final List<ExportResult> list;

  ExportResultList(this.list);

  static ExportResultList fromJson(List<dynamic> json) {
    final list = <ExportResult>[];
    for (var item in json) {
      list.add(ExportResult.fromJson(item as Map<String, dynamic>));
    }
    return ExportResultList(list);
  }

  List<dynamic> toJson() {
    var result = <dynamic>[];
    for (var item in list) {
      result.add(item.toJson());
    }
    return result;
  }
}
