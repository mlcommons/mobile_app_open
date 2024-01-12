import { DataTable } from "../../../sharedComponents/DataTable";
import * as benchmarkTableDataSource from "../services/benchmarksTableDataSource";
import React from "react";
import { BenchmarkResultItem, Result } from "../models/benchmarks.model";

type BenchmarksTableProps = {
  data: Result[];
  onClick: (data: BenchmarkResultItem) => void;
};

const BenchmarksTable = ({ data, onClick }: BenchmarksTableProps) => {
  return (
    <DataTable
      columns={benchmarkTableDataSource.columns()}
      data={benchmarkTableDataSource.data(data)}
      onClick={onClick}
    />
  );
};

export default BenchmarksTable;
