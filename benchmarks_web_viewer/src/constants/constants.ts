export const QUERY_KEY = {
  BENCHMARKS: "benchmarks",
  USER: "user",
};

export class StringValue {
  static unknown: string = "unknown";
  static apple: string = "Apple";
}

export class BenchmarkId {
  static readonly imageClassification = "image_classification";
  static readonly objectDetection = "object_detection";
  static readonly imageSegmentationV2 = "image_segmentation_v2";
  static readonly naturalLanguageProcessing = "natural_language_processing";
  static readonly imageClassificationOffline = "image_classification_offline";
  static readonly superResolution = "super_resolution";

  static readonly allIds = [
    BenchmarkId.imageClassification,
    BenchmarkId.objectDetection,
    BenchmarkId.imageSegmentationV2,
    BenchmarkId.naturalLanguageProcessing,
    BenchmarkId.imageClassificationOffline,
    BenchmarkId.superResolution,
  ];
}
