import * as firebaseStorageService from "../../../services/firebaseStorageService";
import axios from "axios";
import { convertBlobToJson } from "../../../utilities/utilities";
import { BenchmarkResultType } from "../models/benchmark";

export const listResults = async (userId: string | undefined) => {
  if (userId) {
    return firebaseStorageService.listItems(`user/${userId}/result`);
  }
  return Promise.resolve(null);
};

export const downloadResults = async (
  userId: string | undefined,
  benchmarkResults: BenchmarkResultType[] | undefined,
) => {
  const results: BenchmarkResultType[] = benchmarkResults
    ? [...benchmarkResults]
    : [];
  if (userId) {
    const items = await firebaseStorageService.listItems(
      `user/${userId}/result`,
    );
    const benchmarkResultsIds = benchmarkResults?.map(
      (benchmarkResult) => benchmarkResult.meta.uuid,
    );
    for (const item of items) {
      const resultUuid = item.name.replace(".json", "").split("_")[1];
      if (benchmarkResultsIds?.includes(resultUuid)) {
        continue;
      }
      const itemUrl = await firebaseStorageService.getDownloadUrl(item.path);
      const jsonObj = await downloadFile(itemUrl);
      results.push(jsonObj as BenchmarkResultType);
    }
  }
  return results;
};

const downloadFile = async (fileUrl: string) => {
  axios.defaults.headers.post["Access-Control-Allow-Origin"] = "*";
  return axios
    .get(fileUrl, {
      responseType: "blob",
    })
    .then((result) => {
      return convertBlobToJson(result.data);
    });
};

export const fetchBenchmark = async (benchmarkId: string) => {
  return null;
};
