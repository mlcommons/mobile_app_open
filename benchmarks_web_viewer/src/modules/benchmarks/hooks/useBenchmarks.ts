import * as benchmarksDataService from "../services/benchmarksDataService";
import {
  keepPreviousData,
  useQuery,
  useQueryClient,
} from "@tanstack/react-query";
import { BenchmarkResult } from "../models/benchmarks.model";
import { QUERY_KEY } from "../../../constants/constants";
import { useFilters } from "../../filters/hooks/useFilters";

export const useBenchmarks = (userId: string | undefined) => {
  const queryClient = useQueryClient();
  const { resultFilter } = useFilters();
  const benchmarkResults: BenchmarkResult[] | undefined =
    queryClient.getQueryData([QUERY_KEY.BENCHMARKS, userId]);

  return useQuery({
    queryFn: ({ queryKey }) =>
      benchmarksDataService.downloadResults(userId, benchmarkResults),
    queryKey: [QUERY_KEY.BENCHMARKS, userId],
    select: (data) =>
      data
        .map((benchmarkResult) =>
          resultFilter ?
            (resultFilter.match(benchmarkResult) ? benchmarkResult.results : []) :
            benchmarkResult.results,
        )
        .filter((results) => results.length > 0)
        .flat(),
    placeholderData: keepPreviousData,
  });
};
