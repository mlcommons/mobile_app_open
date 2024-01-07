import { BenchmarkRun, Result } from "../models/benchmarks.model";
import { Box, Divider, Heading, Icon, List, ListItem } from "@chakra-ui/react";
import { useUser } from "../../auth/hooks/useUser";
import { useBenchmarks } from "../hooks/useBenchmarks";
import { GrClose } from "react-icons/gr";

type Props = {
  onClose: any;
  benchmarkId: string;
  isOpen?: boolean;
};

const BenchmarkDrawerContent = ({ onClose, benchmarkId, isOpen }: Props) => {
  const user = useUser();
  const { getBenchmarkById } = useBenchmarks(user?.uid);

  const benchmark = getBenchmarkById(benchmarkId);

  const makeBody = (benchmark: Result) => {
    const mainInfo = makeMainInfo(benchmark);
    const performanceRun = benchmark.performance_run
      ? makePerformanceInfo(benchmark.performance_run)
      : [makeSubHeader("N/A")];
    const accuracyRun = benchmark.accuracy_run
      ? makeAccuracyInfo(benchmark.accuracy_run)
      : [makeSubHeader("N/A")];
    return [
      ...mainInfo,
      <Divider key="divider1" padding={2} />,
      makeHeader("Perf", "Performance run"),
      ...performanceRun,
      <Divider key="divider2" padding={2} />,
      makeHeader("Accuracy", "Accuracy run"),
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
      makeInfo("QPS", perf.throughput?.value ?? "N/A"),
      makeInfo(
        "Run is valid",
        perf.loadgen_info?.validity.toString() ?? "false",
      ),
      makeInfo("Duration", formatDuration(perf.measured_duration)),
      makeInfo("Samples count", perf.measured_samples.toString()),
      makeInfo("Dataset Type", perf.dataset.type),
      makeInfo("Dataset Name", perf.dataset.name),
    ];
  };

  const makeAccuracyInfo = (accuracy: BenchmarkRun) => {
    return [
      makeInfo("Accuracy", accuracy.accuracy?.formatted ?? "N/A"),
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
    <ListItem key="subheader" fontStyle="italic" mb={4}>
      {translationKey}
    </ListItem>
  );

  const formatDuration = (seconds: number) => {
    // Implement formatDuration function as per your requirements
    return `${seconds} seconds`; // Placeholder implementation
  };

  const onCloseDrawer = (e: React.SyntheticEvent) => {
    e.stopPropagation();
    onClose();
  };

  return (
    <Box padding={2}>
      <Heading>{benchmark?.benchmark_name}</Heading>
      {benchmark && <List>{makeBody(benchmark)}</List>}
      <Icon
        as={GrClose}
        fontSize="25px"
        cursor="pointer"
        onClick={onCloseDrawer}
        pos="absolute"
        top="20px"
        right="20px"
      />
    </Box>
  );
};

export default BenchmarkDrawerContent;
