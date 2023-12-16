import { useLocation } from "react-router-dom";
import { BenchmarkRun, Result } from "../models/benchmarks.model";
import { Box, Divider, Heading, List, ListItem } from "@chakra-ui/react";

const BenchmarkDetailsPage = () => {
  const location = useLocation();
  const benchmark: Result = JSON.parse(location.state);

  const makeBody = () => {
    const mainInfo = makeMainInfo(benchmark);
    const performanceRun = benchmark.performance_run
      ? makePerformanceInfo(benchmark.performance_run)
      : [makeSubHeader("resultsNotAvailable")];
    const accuracyRun = benchmark.accuracy_run
      ? makeAccuracyInfo(benchmark.accuracy_run)
      : [makeSubHeader("resultsNotAvailable")];
    return [
      ...mainInfo,
      <Divider key="divider1" padding={2} />,
      makeHeader("Perf", "historyRunDetailsPerfTitle"),
      ...performanceRun,
      <Divider key="divider2" padding={2} />,
      makeHeader("Accuracy", "historyRunDetailsAccuracyTitle"),
      ...accuracyRun,
    ];
  };

  const makeMainInfo = (res: Result) => {
    return [
      makeInfo("Benchmark Name", res.benchmark_name),
      makeInfo("Scenario", res.loadgen_scenario),
      makeInfo("Backend Name", res.backend_info.backend_name),
      makeInfo("Vendor Name", res.backend_info.backend_name),
      makeInfo("Delegate", res.backend_settings.delegate),
      makeInfo("Accelerator", res.backend_info.accelerator_name),
      res.loadgen_scenario === "Offline" &&
        makeInfo("Batch Size", res.backend_settings.batch_size.toString()),
    ];
  };

  const makePerformanceInfo = (perf: BenchmarkRun) => {
    return [
      makeInfo("QPS", perf.throughput?.value ?? "resultsNotAvailable"),
      makeInfo("Valid", perf.loadgen_info?.validity.toString() ?? "false"),
      makeInfo("Duration", formatDuration(perf.measured_duration)),
      makeInfo("Samples", perf.measured_samples.toString()),
      makeInfo("Dataset Type", perf.dataset.type),
      makeInfo("Dataset Name", perf.dataset.name),
    ];
  };

  const makeAccuracyInfo = (accuracy: BenchmarkRun) => {
    return [
      makeInfo(
        "Accuracy",
        accuracy.accuracy?.formatted ?? "resultsNotAvailable",
      ),
      makeInfo("Duration", formatDuration(accuracy.measured_duration)),
      makeInfo("Samples", accuracy.measured_samples.toString()),
      makeInfo("Dataset Type", accuracy.dataset.type),
      makeInfo("Dataset Name", accuracy.dataset.name),
    ];
  };

  const makeInfo = (label: string, value: string | number) => (
    <ListItem key={label}>
      <strong>{label}:</strong> {value}
    </ListItem>
  );

  const makeHeader = (title: string, translationKey: string) => (
    <Heading key={title} size="lg" mt={4}>
      {translationKey}
    </Heading>
  );

  const makeSubHeader = (translationKey: string) => (
    <Heading key="subheader" fontStyle="italic" mb={4}>
      {translationKey}
    </Heading>
  );

  const formatDuration = (seconds: number) => {
    // Implement formatDuration function as per your requirements
    return `${seconds} seconds`; // Placeholder implementation
  };

  console.log("body:", makeBody());

  return (
    <Box padding={2}>
      <Heading>{benchmark.benchmark_name}</Heading>
      <List>{makeBody()}</List>
    </Box>
  );
};

export default BenchmarkDetailsPage;
