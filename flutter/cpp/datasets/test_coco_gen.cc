#include <iostream>

#include "flutter/cpp/datasets/coco_gen_utils/clip_score.h"

//int main(int argc, char *argv[]) {
//  std::string the_file = "/Users/anh/dev/mlcommons/mobile_app_open/flutter/cpp/datasets/val_toke_ids_and_clip_scores.tfrecord";
//  std::cout << "Loading: " << the_file << "\n";
//
//  mlperf::mobile::TFRecordReader reader(the_file);
//  for (int i; i < 10; i++) {
//    tensorflow::tstring r = reader.ReadRecord(i);
//    std::cout << i << ": " << r.size() << ", " << reader.Size() << "\n";
//    mlperf::mobile::CaptionRecord *d = new mlperf::mobile::CaptionRecord(r);
//    // d->dump();
//    auto ids = d->get_tokenized_ids();
//    for (int i = 0; i < 77; i++) {
//      if (i == 0)
//        std::cout << "[" << ids[i] << ", ";
//      else if (i == 76)
//        std::cout << ids[i] << "]\n";
//      else
//        std::cout << ids[i] << ", ";
//    }
//  }
//  exit(0);
//}


int main() {
  // Example usage
    std::string model_path = "/Users/anh/Downloads/stable-diffusion-android/clip_score_on_android/clip_model.tflite";
    std::cout << "Model path: " << model_path << "\n";
    mlperf::mobile::CLIPScore clip_score(model_path);

    // Dummy inputs
    std::vector<int32_t> attention_mask(77, 0.0f);  // Adjust size accordingly
    std::vector<int32_t> input_ids(77, 0.0f);  // Adjust size accordingly
    std::vector<float> pixel_values(3 * 224 * 224, 1.0f);  // Example for 224x224 RGB image

    // Outputs
    std::vector<float> logits_per_text;
    std::vector<float> logits_per_image;

    // Make a prediction
    clip_score.predict(input_ids, attention_mask, pixel_values, logits_per_text, logits_per_image);

    // Print the results (for demonstration purposes)
    std::cout << "Logits per Text: ";
    for (const auto& val : logits_per_text) {
      std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << "Logits per Image: ";
    for (const auto& val : logits_per_image) {
      std::cout << val << " ";
    }
    std::cout << std::endl;

  return 0;
}
