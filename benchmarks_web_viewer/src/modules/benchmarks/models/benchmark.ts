export type BenchmarkListItemType = {
  name: string;
  path: string;
};

export type BenchmarkResultType = {
  meta: MetaType;
  results: ResultType[];
  environment_info: EnvironmentInfoType;
  build_info: BuildInfoType;
};

export type MetaType = {
  uuid: string;
  upload_date: string;
};

export type ResultType = {
  benchmark_id: string;
  benchmark_name: string;
  loadgen_scenario: string;
  backend_settings: BackendSettingsType;
  backend_info: BackendInfoType;
  performance_run: BenchmarkRunType;
  accuracy_run: BenchmarkRunType;
  min_duration: number;
  min_samples: number;
};

export type BackendSettingsType = {
  accelerator_code: string;
  accelerator_desc: string;
  framework: string;
  delegate: string;
  model_path: string;
  batch_size: string;
  extra_settings: [any];
};

export type BackendInfoType = {
  filename: string;
  vendor_name: string;
  backend_name: string;
  accelerator_name: string;
};

export type BenchmarkRunType = {
  throughput: ThroughputType;
  accuracy: AccuracyType;
  dataset: DataSetType;
  measured_duration: number;
  measured_samples: number;
  start_datetime: string;
  loadgen_info: any;
};

export type ThroughputType = {
  value: number;
};

export type AccuracyType = {
  normilezed: number;
  formatted: string;
};

export type DataSetType = {
  name: string;
  type: string;
  data_path: string;
  groundtruth_path: string;
};

export type EnvironmentInfoType = {
  platform: string;
  value: any;
};

export type BuildInfoType = {
  version: string;
  build_number: string;
  build_date: string;
  official_release_flag: boolean;
  dev_test_flag: boolean;
  backend_list: string[];
  git_branch: string;
  git_commit: string;
  git_dirty_flag: string;
};
