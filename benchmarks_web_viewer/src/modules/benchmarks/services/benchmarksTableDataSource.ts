import { BenchmarkResultItem, Result } from "../models/benchmarks.model";
import { formatDateTime } from "../../../utilities/timeUtils";
import { createColumnHelper } from "@tanstack/react-table";

const itemScore = (item: Result) => {
  const throughput = item.performance_run?.throughput;
  const accuracy = item.accuracy_run?.accuracy;
  if (throughput != null) {
    return throughput.value.toFixed(2);
  }
  if (accuracy != null) {
    return accuracy.formatted;
  }
  return "unknown";
};

const itemDateTime = (item: Result) => {
  const prDateTime = item.performance_run?.start_datetime;
  const arDateTime = item.accuracy_run?.start_datetime;
  if (prDateTime != null) {
    return formatDateTime(new Date(prDateTime));
  }
  if (arDateTime != null) {
    return formatDateTime(new Date(arDateTime));
  }
  return "unknown";
};

export const data = (items: Result[]): BenchmarkResultItem[] => {
  return items?.map((item) => {
    return {
      id: item.id,
      benchmarkId: item.benchmark_id,
      benchmarkName: item.benchmark_name,
      backendName: item.backend_info.backend_name,
      delegateName: item.backend_settings.delegate,
      acceleratorName: item.backend_info.accelerator_name,
      date: itemDateTime(item),
      score: itemScore(item),
      platform: item.platform
    };
  });
};

export const columns = () => {
  const columnHelper = createColumnHelper<BenchmarkResultItem>();
  return [
    columnHelper.accessor("platform", {
      cell: (info) => info.getValue(),
      header: "Platform",
    }),
    columnHelper.accessor("benchmarkName", {
      cell: (info) => info.getValue(),
      header: "Benchmark Name",
    }),
    columnHelper.accessor("backendName", {
      cell: (info) => info.getValue(),
      header: "Backend Name",
    }),
    columnHelper.accessor("delegateName", {
      cell: (info) => info.getValue(),
      header: "Delegate Name",
    }),
    columnHelper.accessor("acceleratorName", {
      cell: (info) => info.getValue(),
      header: "Accelerator Name",
    }),
    columnHelper.accessor("date", {
      cell: (info) => info.getValue(),
      header: "Date",
    }),
    columnHelper.accessor("score", {
      cell: (info) => info.getValue(),
      header: "Item Score",
    }),
  ];
};
