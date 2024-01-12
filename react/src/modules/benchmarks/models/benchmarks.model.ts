export type BenchmarkResultItem = {
  id: string;
  benchmarkId: string;
  benchmarkName?: string;
  backendName?: string;
  delegateName?: string;
  acceleratorName?: string;
  date?: string;
  score?: string;
  platform?: string;
};

export type BenchmarkResult = {
  meta: Meta;
  results: Result[];
  environment_info: EnvironmentInfo;
  build_info: BuildInfo;
};

export type Meta = {
  uuid: string;
  upload_date: string;
  creation_date: string;
};

export type Result = {
  id: string;
  benchmark_id: string;
  benchmark_name: string;
  loadgen_scenario: string;
  backend_settings: BackendSettings;
  backend_info: BackendInfo;
  performance_run: BenchmarkRun;
  accuracy_run: BenchmarkRun;
  min_duration: number;
  min_samples: number;
  platform: string;
};

export type BackendSettings = {
  accelerator_code: string;
  accelerator_desc: string;
  framework: string;
  delegate: string;
  model_path: string;
  batch_size: string;
  extra_settings: [any];
};

export type BackendInfo = {
  filename: string;
  vendor_name: string;
  backend_name: string;
  accelerator_name: string;
};

export type BenchmarkRun = {
  throughput: Throughput;
  accuracy: Accuracy;
  dataset: DataSet;
  measured_duration: number;
  measured_samples: number;
  start_datetime: string;
  loadgen_info: any;
};

export type Throughput = {
  value: number;
};

export type Accuracy = {
  normilezed: number;
  formatted: string;
};

export type DataSet = {
  name: string;
  type: string;
  data_path: string;
  groundtruth_path: string;
};

export type EnvironmentInfo = {
  platform: string;
  value: {
    android: {
      os_version: string;
      manufacturer: string;
      model_code: string;
      model_name: string;
      board_code: string;
      proc_cpuinfo_soc_name: string;
      props: any[];
    };
    ios: {
      os_version: string;
      model_code: string;
      model_name: string;
      soc_name: string;
    };
    windows: {
      os_version: string;
      cpu_full_name: string;
    };
  };
};

export type BuildInfo = {
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
