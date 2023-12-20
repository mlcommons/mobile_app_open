import React from "react";
import { Box, Text, ListItem } from "@chakra-ui/react";
import { formatDateTime } from "../../../utilities/timeUtils";
import { Result } from "../models/benchmarks.model";

type BenchmarkListItemProps = {
  item: Result;
  tapHandler?: () => void;
};
const BenchmarkListItem: React.FC<BenchmarkListItemProps> = ({
  item,
  tapHandler,
}) => {
  const itemScore = () => {
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

  const itemDateTime = () => {
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

  const itemAdditionalInfo = () => {
    const backendName = item.backend_info.backend_name;
    const delegateName = item.backend_settings.delegate;
    const acceleratorName = item.backend_info.accelerator_name;
    return `${backendName} | ${delegateName} | ${acceleratorName}`;
  };

  return (
    <ListItem
      onClick={tapHandler}
      cursor="pointer"
      display="flex"
      justifyContent="space-between"
      padding={2}
    >
      <Box>
        <Text fontWeight="bold">{item.benchmark_name}</Text>
        <Text fontWeight="normal" lineHeight="1.4">
          {itemAdditionalInfo()}
        </Text>
        <Text fontWeight="normal" lineHeight="1.4">
          {itemDateTime()}
        </Text>
      </Box>
      <Text fontWeight="bold">{itemScore()}</Text>
    </ListItem>
  );
};

export { BenchmarkListItem };
