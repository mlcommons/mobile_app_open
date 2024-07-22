#include <iostream>

#include "flutter/cpp/datasets/coco_gen.h"

int main(int argc, char *argv[]) {
  std::string the_file = "val_toke_ids_and_clip_scores.tfrecord";

  mlperf::mobile::TFRecordReader reader(the_file);
  for (int i; i < 10; i++) {
    tensorflow::tstring r = reader.ReadRecord(i);
    std::cout << i << ": " << r.size() << ", " << reader.Size() << "\n";
    mlperf::mobile::CaptionRecord *d = new mlperf::mobile::CaptionRecord(r);
    // d->dump();
    auto ids = d->get_tokenized_ids();
    for (int i = 0; i < 77; i++) {
      if (i == 0)
        std::cout << "[" << ids[i] << ", ";
      else if (i == 76)
        std::cout << ids[i] << "]\n";
      else
        std::cout << ids[i] << ", ";
    }
  }
  exit(0);
}
