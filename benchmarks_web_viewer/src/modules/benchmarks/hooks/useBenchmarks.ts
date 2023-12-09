import * as benchmarksDataService from "../services/benchmarksDataService";
import {
  keepPreviousData,
  useQuery,
  useQueryClient,
} from "@tanstack/react-query";
import { QUERY_KEY } from "../../../constants/queryKeys";
import { BenchmarkResultType } from "../models/benchmark";

export const useBenchmarks = (userId: string | undefined) => {
  const queryClient = useQueryClient();
  const benchmarkResults: BenchmarkResultType[] | undefined =
    queryClient.getQueryData([QUERY_KEY.BENCHMARKS, userId]);

  return useQuery({
    queryFn: ({ queryKey }) =>
      benchmarksDataService.downloadResults(userId, benchmarkResults),
    queryKey: [QUERY_KEY.BENCHMARKS, userId],
    select: (data) =>
      data.flatMap((benchmarkResult) => benchmarkResult.results),
    placeholderData: keepPreviousData,
  });
};
