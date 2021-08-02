/* Copyright 2020 The MLPerf Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef MLPERF_DATASETS_SQUAD_UTILS_TFRECORD_READER_H_
#define MLPERF_DATASETS_SQUAD_UTILS_TFRECORD_READER_H_

#include "tensorflow/core/lib/io/record_reader.h"

namespace mlperf {
namespace mobile {

// TFRecordReader is similar to tensorflow::io::RecordReader. However, it allows
// users to get the number of records in the file and random access to a record
// using its index instead of offset.
class TFRecordReader {
 public:
  TFRecordReader(const std::string& filename) {
    TF_CHECK_OK(tensorflow::Env::Default()->NewRandomAccessFile(
        filename, &random_access_file_));
    auto options =
        tensorflow::io::RecordReaderOptions::CreateRecordReaderOptions("ZLIB");
    reader_.reset(
        new tensorflow::io::RecordReader(random_access_file_.get(), options));

    tensorflow::uint64 id = 0, offset = 0, noffset = 0;
    tensorflow::tstring record;
    while (reader_->ReadRecord(&noffset, &record).ok()) {
      index_to_offset_[id++] = offset;
      offset = noffset;
    }
  }

  tensorflow::tstring ReadRecord(int idx) {
    if (idx >= index_to_offset_.size()) {
      LOG(FATAL) << "Sample index out of bound";
    }
    tensorflow::tstring record;
    tensorflow::uint64 offset = index_to_offset_[idx];
    if (!reader_->ReadRecord(&offset, &record).ok()) {
      LOG(FATAL) << "Failed to read tfrecord file";
    }
    return record;
  }

  uint32_t Size() { return index_to_offset_.size(); }

 private:
  std::unique_ptr<tensorflow::RandomAccessFile> random_access_file_;
  std::unique_ptr<tensorflow::io::RecordReader> reader_;
  // Map the Record index to its offset.
  std::unordered_map<uint32_t, tensorflow::uint64> index_to_offset_;
};
}  // namespace mobile
}  // namespace mlperf

#endif  // MLPERF_DATASETS_SQUAD_UTILS_TFRECORD_READER_H_
