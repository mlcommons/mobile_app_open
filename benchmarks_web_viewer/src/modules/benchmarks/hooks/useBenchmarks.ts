import * as benchmarksDataService from "../services/benchmarksDataService";
import {
  keepPreviousData,
  useQuery,
  useQueryClient,
} from "@tanstack/react-query";
import { BenchmarkResult, Result } from "../models/benchmarks.model";
import { QUERY_KEY } from "../../../constants/constants";
import { useFilters } from "../../filters/hooks/useFilters";

export const useBenchmarks = (userId: string | undefined) => {
  const queryClient = useQueryClient();
  const { resultFilter } = useFilters();
  const benchmarkResults: BenchmarkResult[] | undefined =
    queryClient.getQueryData([QUERY_KEY.BENCHMARKS, userId]);

  const {
    data: benchmarks,
    isLoading,
    error,
  } = useQuery({
    queryFn: ({ queryKey }) =>
      benchmarksDataService.downloadResults(userId, benchmarkResults),
    queryKey: [QUERY_KEY.BENCHMARKS, userId],
    select: (data) =>
      data
        .map((benchmarkResult) =>
          resultFilter
            ? resultFilter.match(benchmarkResult)
              ? benchmarkResult.results
              : []
            : benchmarkResult.results,
        )
        .filter((results) => results.length > 0)
        .flat()
        .filter((result: Result) => resultFilter.matchBenchmark(result))
        .map((result) => {
          return { ...result, id: `${composeId(result)}` };
        }),
    placeholderData: keepPreviousData,
  });

  const getBenchmarkById = (id: string) => {
    return benchmarks?.find((benchmark) => benchmark.id === id);
  };

  return { benchmarks, getBenchmarkById, isLoading, error };
};

const composeId = (item: Result) => {
  const prDate = item.performance_run?.start_datetime;
  const arDate = item.accuracy_run?.start_datetime;
  return `${item.benchmark_id}_${prDate || arDate}`;
};
