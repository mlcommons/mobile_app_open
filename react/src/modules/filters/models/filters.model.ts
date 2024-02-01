import {
  BenchmarkResult,
  EnvironmentInfo,
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
  fromCreationDate: Date | null;
  toCreationDate: Date | null;
  platform: string | null;
  deviceModel: string | null;
  backend: string | null;
  manufacturer: string | null;
  soc: string | null;
  benchmarkId: string | null;

  constructor(filter: ResultFilterType = {}) {
    this.fromCreationDate = filter.fromCreationDate ?? null;
    this.toCreationDate = filter.toCreationDate ?? null;
    this.platform = filter.platform ?? null;
    this.deviceModel = filter.deviceModel ?? null;
    this.backend = filter.backend ?? null;
    this.manufacturer = filter.manufacturer ?? null;
    this.soc = filter.soc ?? null;
    this.benchmarkId = filter.benchmarkId ?? null;
  }

  match(result: BenchmarkResult) {
    const { deviceModel, manufacturer, soc } = this.getDeviceDetails(
      result.environment_info,
    );
    const resultCreationDate = new Date(result.meta.creation_date);
    const resultBackend = result.results[0].backend_info.filename;

    const platformMatches =
      this.platform === null ||
      this.platform === result.environment_info.platform;
    const backendMatches =
      this.backend === null || this.backend === resultBackend;

    return (
      this.isDateWithinRange(
        resultCreationDate,
        this.fromCreationDate,
        this.toCreationDate,
      ) &&
      platformMatches &&
      this.stringMatches(deviceModel, this.deviceModel) &&
      this.stringMatches(manufacturer, this.manufacturer) &&
      this.stringMatches(soc, this.soc) &&
      backendMatches
    );
  }

  getDeviceDetails(envInfo: EnvironmentInfo) {
    switch (envInfo.platform) {
      case Platform.Android:
        return {
          deviceModel: envInfo.value.android?.model_name ?? StringValue.unknown,
          manufacturer:
            envInfo.value.android?.manufacturer ?? StringValue.unknown,
          soc:
            envInfo.value.android?.proc_cpuinfo_soc_name ?? StringValue.unknown,
        };
      case Platform.iOS:
        return {
          deviceModel: envInfo.value.ios?.model_name ?? StringValue.unknown,
          manufacturer: "Apple",
          soc: envInfo.value.ios?.soc_name ?? StringValue.unknown,
        };
      case Platform.Windows:
        return {
          deviceModel:
            envInfo.value.windows?.cpu_full_name ?? StringValue.unknown,
          manufacturer: StringValue.unknown,
          soc: envInfo.value.windows?.cpu_full_name ?? StringValue.unknown,
        };
      default:
        return {
          deviceModel: StringValue.unknown,
          manufacturer: StringValue.unknown,
          soc: StringValue.unknown,
        };
    }
  }

  // Helper function for date comparison
  isDateWithinRange(date: Date, fromDate: Date | null, toDate: Date | null) {
    const oneDay = 24 * 60 * 60 * 1000;
    const toDateAdjusted = toDate ? new Date(toDate.getTime() + oneDay) : null;
    return (
      (!fromDate || date > fromDate) &&
      (!toDateAdjusted || date < toDateAdjusted)
    );
  }

  // Helper function for string matching
  stringMatches(value: string, pattern: string | null) {
    return value.toLowerCase().includes((pattern ?? "").toLowerCase());
  }

  matchBenchmark(result: Result): boolean {
    return this.benchmarkId == null
      ? true
      : result.benchmark_id === this.benchmarkId;
  }
}
