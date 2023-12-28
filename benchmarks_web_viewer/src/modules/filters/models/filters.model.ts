import {
  BenchmarkResult,
  Result,
} from "../../benchmarks/models/benchmarks.model";
import { StringValue } from "../../../constants/constants";

export enum Platform {
  Android = "android",
  iOS = "ios",
  Windows = "windows",
}

export interface ResultFilterType {
  fromCreationDate?: Date | null;
  toCreationDate?: Date | null;
  platform?: string | null;
  deviceModel?: string | null;
  backend?: string | null;
  manufacturer?: string | null;
  soc?: string | null;
  benchmarkId?: string | null;
}

export class ResultFilter implements ResultFilterType {
  fromCreationDate?: Date | null;
  toCreationDate?: Date | null;
  platform?: string | null;
  deviceModel?: string | null;
  backend?: string | null;
  manufacturer?: string | null;
  soc?: string | null;
  benchmarkId?: string | null;

  constructor(filter: ResultFilterType = {}) {
    this.fromCreationDate = filter.fromCreationDate || null;
    this.toCreationDate = filter.toCreationDate || null;
    this.platform = filter.platform || null;
    this.deviceModel = filter.deviceModel || null;
    this.backend = filter.backend || null;
    this.manufacturer = filter.manufacturer || null;
    this.soc = filter.soc || null;
    this.benchmarkId = filter.benchmarkId || null;
  }

  match(result: BenchmarkResult): boolean {
    let resultDeviceModel: string | null = null;
    let resultManufacturer: string | null = null;
    let resultSoc: string | null = null;
    const envInfo = result.environment_info;

    switch (envInfo.platform) {
      case Platform.Android:
        resultDeviceModel = envInfo.value.android?.model_name || null;
        resultManufacturer = envInfo.value.android?.manufacturer || null;
        resultSoc = envInfo.value.android?.proc_cpuinfo_soc_name || null;
        break;

      case Platform.iOS:
        resultDeviceModel = envInfo.value.ios?.model_name || null;
        resultManufacturer = "Apple";
        resultSoc = envInfo.value.ios?.soc_name || null;
        break;

      case Platform.Windows:
        resultDeviceModel = envInfo.value.windows?.cpu_full_name || null;
        resultManufacturer = StringValue.unknown;
        resultSoc = envInfo.value.windows?.cpu_full_name || null;
        break;
    }

    resultDeviceModel = resultDeviceModel || StringValue.unknown;
    resultManufacturer = resultManufacturer || StringValue.unknown;
    resultSoc = resultSoc || StringValue.unknown;

    const resultCreationDate = new Date(result.meta.creation_date);
    const resultBackend = result.results[0].backend_info.filename;
    const resultPlatform = envInfo.platform;

    const oneDay: number = 24 * 60 * 60 * 1000;

    const fromCreationDateMatched =
      this.fromCreationDate === null ||
      this.fromCreationDate === undefined ||
      resultCreationDate > this.fromCreationDate;
    const toCreationDateMatched =
      this.toCreationDate === null ||
      this.toCreationDate === undefined ||
      resultCreationDate < new Date(this.toCreationDate.getTime() + oneDay);

    const platformMatched =
      this.platform === null || this.platform === resultPlatform;
    const deviceModelMatched = resultDeviceModel.includes(
      (this.deviceModel || "").toLowerCase(),
    );
    const backendMatched =
      this.backend === null || this.backend === resultBackend;
    const manufacturerMatched = resultManufacturer.includes(
      (this.manufacturer || "").toLowerCase(),
    );
    const socMatched = resultSoc.includes((this.soc || "").toLowerCase());

    return (
      fromCreationDateMatched &&
      toCreationDateMatched &&
      platformMatched &&
      deviceModelMatched &&
      backendMatched &&
      manufacturerMatched &&
      socMatched
    );
  }

  matchBenchmark(result: Result): boolean {
    return this.benchmarkId == null
      ? true
      : result.benchmark_id === this.benchmarkId;
  }
}
