import { useBenchmarks } from "../hooks/useBenchmarks";
import { List } from "@chakra-ui/react";
import { useNavigate } from "react-router-dom";
import { ResultType } from "../models/benchmark";
import { BenchmarkListItem } from "./benchmarkListItem";
import React from "react";
import { useUser } from "../../auth/hooks/useUser";

const BenchmarkListPage = () => {
  const navigate = useNavigate();

  const user = useUser();
  const { data } = useBenchmarks(user?.uid);

  const renderItem = (item: ResultType) => {
    return (
      <BenchmarkListItem
        item={item}
        tapHandler={() => {
          navigate(`/benchmarks/${item.benchmark_id}`, {
            state: JSON.stringify(item),
          });
        }}
      />
    );
  };

  return <List>{data?.map((item) => renderItem(item))}</List>;
};

export default BenchmarkListPage;
