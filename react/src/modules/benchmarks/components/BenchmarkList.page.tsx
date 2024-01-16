import { useBenchmarks } from "../hooks/useBenchmarks";
import { ChakraProvider, useDisclosure } from "@chakra-ui/react";
import React, { useState } from "react";
import { useUser } from "../../auth/hooks/useUser";
import BenchmarkTableDrawer from "./BenchmarkTableDrawer";
import BenchmarksTable from "./BenchmarksTable";
import { BenchmarkResultItem } from "../models/benchmarks.model";
import CenterSpinner from "../../../sharedComponents/CenterSpinner";

const BenchmarkListPage = () => {
  const { isOpen, onClose, onOpen } = useDisclosure();

  const user = useUser();
  const { benchmarks, isLoading } = useBenchmarks(user?.uid);

  const [benchmarkId, setBenchmarkId] = useState<string | null>();
  const onInternalClose = () => {
    setBenchmarkId(null);
    onClose();
  };

  const onInternalOpen = (data: BenchmarkResultItem) => {
    setBenchmarkId(data.id);
    onOpen();
  };

  return (
    <ChakraProvider>
      {benchmarks && (
        <BenchmarksTable data={benchmarks} onClick={onInternalOpen} />
      )}
      {isLoading && <CenterSpinner />}
      {benchmarkId && (
        <BenchmarkTableDrawer
          benchmarkId={benchmarkId}
          isOpen={isOpen}
          onClose={onInternalClose}
        />
      )}
    </ChakraProvider>
  );
};

export default BenchmarkListPage;
