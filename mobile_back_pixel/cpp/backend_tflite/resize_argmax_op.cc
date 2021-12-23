/* Copyright 2021 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <string.h>

#include <algorithm>
#include <memory>
#include <thread>
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#include "resize_argmax_op.h"
#include "tensorflow/lite/core/api/profiler.h"
#include "tensorflow/lite/kernels/cpu_backend_context.h"
#include "tensorflow/lite/kernels/cpu_backend_threadpool.h"
#include "tensorflow/lite/kernels/kernel_util.h"

constexpr int kInputTensor = 0;
constexpr int kSizeTensor = 1;
constexpr int kAxis = 2;
constexpr int kOutputTensor = 0;

using namespace tflite;

namespace {

inline int ArgMaxVector(const uint8_t* input_data, int size) {
  int32_t max_index = 0;
  uint8_t max_value = input_data[0];
  int32_t i = 0;
#ifdef __ARM_NEON
  constexpr int VECTOR_SIZE = 16;
  if (size >= VECTOR_SIZE) {
    uint8x16_t max_value_u8x16;
    for (; i <= size - VECTOR_SIZE; i += VECTOR_SIZE) {
      max_value_u8x16 = vld1q_u8(input_data + i);
      uint8_t max_from_vec;
      max_from_vec = vmaxvq_u8(max_value_u8x16);
      if (max_from_vec > max_value) {
        max_value = max_from_vec;
        max_index = i;
      }
    }
  }
  for (int start_idx = max_index; start_idx < max_index + VECTOR_SIZE;
       start_idx++) {
    if (input_data[start_idx] == max_value) {
      max_index = start_idx;
      break;
    }
  }

#endif  // __aarch64__
  // Leftover loop.
  for (; i < size; ++i) {
    const uint8_t curr_value = input_data[i];
    if (curr_value > max_value) {
      max_value = curr_value;
      max_index = i;
    }
  }

  return max_index;
}

inline void fill_bilinear_row(uint32_t* row, uint32_t left_val,
                              uint32_t right_val) {
  row[0] = left_val;
  row[1] = (left_val * 3 + right_val) / 4;
  row[2] = (left_val + right_val) / 2;
  row[3] = (left_val + right_val * 3) / 4;
}

#ifdef __ARM_NEON
constexpr uint32_t left_multipliers[] = {4, 3, 2, 1};
constexpr uint32_t right_multipliers[] = {0, 1, 2, 3};

inline uint32x4_t fill_bilinear_row_simd(uint32_t left_val,
                                         uint32_t right_val) {
  const uint32x4_t left_multipliers_simd = vld1q_u32(left_multipliers);
  const uint32x4_t right_multipliers_simd = vld1q_u32(right_multipliers);

  uint32x4_t left_top_simd = vmulq_n_u32(left_multipliers_simd, left_val);
  uint32x4_t top_row_simd =
      vmlaq_n_u32(left_top_simd, right_multipliers_simd, right_val);
  // shift by 2 = divide by 4
  return vshrq_n_u32(top_row_simd, 2);
}

#endif

void resize_argmax_task_4x(uint8_t* input, int input_width, int input_height,
                           int input_argmax_depth, int32_t* output,
                           int output_width, int output_height, int start_row,
                           int row_count, int move,
                           tflite::Profiler* profiler) {
  TFLITE_SCOPED_TAGGED_DEFAULT_PROFILE(profiler, "resize_argmax_task_4x");
  uint8_t* in_ptr = input + start_row * input_width * input_argmax_depth;
  int32_t* out_ptr = output + start_row * 4 * output_width;
  int cnt = 0;
  for (int row = start_row; row < start_row + row_count; row++) {
    for (int col = 0; col < input_width; col++) {
      int top_left;
      int top_right;
      int bottom_left;
      int bottom_right;
      top_left = *out_ptr;
      if (row != input_height - 1 && col != input_width - 1) {
        if ((row + 1 % move) > 0) {
          if (col == 0) {
            *(out_ptr + output_width * 4) = ArgMaxVector(
                in_ptr + input_width * input_argmax_depth, input_argmax_depth);
          }
          *(out_ptr + output_width * 4 + 4) = ArgMaxVector(
              in_ptr + input_width * input_argmax_depth + input_argmax_depth,
              input_argmax_depth);
        }

        top_right = *(out_ptr + 4);
        bottom_left = *(out_ptr + output_width * 4);
        bottom_right = *(out_ptr + output_width * 4 + 4);
      }
      if ((row == input_height - 1 || col == input_width - 1) ||
          (top_left == top_right && top_right == bottom_left &&
           bottom_left == bottom_right)) {
        cnt++;
#ifdef __ARM_NEON
        int32_t* temp_out = out_ptr;
        int32x4_t f = vdupq_n_s32(top_left);
        for (int square_row = 0; square_row < 4; square_row++) {
          vst1q_s32(temp_out, f);
          temp_out += output_width;
        }
#else
        for (int i = 1; i <= 3; ++i) {
          out_ptr[i] = top_left;
        }
        for (int i = 1; i <= 3; ++i) {
          memcpy(out_ptr + i * output_width, out_ptr, sizeof(int32_t) * 4);
        }
#endif
      } else {
        uint8_t* top_right_col = in_ptr + input_argmax_depth;
        uint8_t* top_left_col = in_ptr;
        uint8_t* bot_right_col =
            in_ptr + input_width * input_argmax_depth + input_argmax_depth;
        uint8_t* bot_left_col = in_ptr + input_width * input_argmax_depth;

#ifdef __ARM_NEON
        uint32x4_t max_values[4];
        for (int i = 0; i < 4; i++) {
          max_values[i] = vdupq_n_u32(0);
        }
        int32x4_t max_indices[4];
        for (int i = 0; i < 4; i++) {
          max_indices[i] = vdupq_n_s32(0);
        }
#else
        uint32_t max_values[16] = {0};
        int32_t max_indices[16] = {0};
#endif
        for (int depth = 0; depth < input_argmax_depth; depth++) {
#ifdef __ARM_NEON
          uint32x4_t simd_rows[5];
          simd_rows[0] = fill_bilinear_row_simd(*top_left_col, *top_right_col);
          simd_rows[4] = fill_bilinear_row_simd(*bot_left_col, *bot_right_col);

          uint32x4_t row2 = vmlaq_n_u32(simd_rows[4], simd_rows[0], 3);
          simd_rows[1] = vshrq_n_u32(row2, 2);

          uint32x4_t row3 = vaddq_u32(simd_rows[0], simd_rows[4]);
          simd_rows[2] = vshrq_n_u32(row3, 1);

          uint32x4_t row4 = vmlaq_n_u32(simd_rows[0], simd_rows[4], 3);
          simd_rows[3] = vshrq_n_u32(row4, 2);

          int32x4_t depth_simd = vdupq_n_s32(depth);
          for (int i = 0; i < 4; i++) {
            uint32x4_t mask = vcgtq_u32(simd_rows[i], max_values[i]);
            max_values[i] = vbslq_u32(mask, simd_rows[i], max_values[i]);
            max_indices[i] = vbslq_s32(mask, depth_simd, max_indices[i]);
          }
#else
          uint32_t bilinear_values[20];
          fill_bilinear_row(bilinear_values, *top_left_col, *top_right_col);
          fill_bilinear_row(bilinear_values + 16, *bot_left_col,
                            *bot_right_col);

          for (int i = 0; i < 4; i++) {
            bilinear_values[i + 4] =
                (bilinear_values[i] * 3 + bilinear_values[i + 16]) / 4;
            bilinear_values[i + 8] =
                (bilinear_values[i] + bilinear_values[i + 16]) / 2;
            bilinear_values[i + 12] =
                (bilinear_values[i] + bilinear_values[i + 16] * 3) / 4;
          }

          for (int i = 0; i < 16; i++) {
            if (bilinear_values[i] > max_values[i]) {
              max_indices[i] = depth;
              max_values[i] = bilinear_values[i];
            }
          }
#endif
          top_right_col++;
          top_left_col++;
          bot_right_col++;
          bot_left_col++;
        }  // for depth
#ifdef __ARM_NEON
        int32_t* temp_out_ptr = out_ptr;
        for (int square_row = 0; square_row < 4; square_row++) {
          vst1q_s32(temp_out_ptr, max_indices[square_row]);
          temp_out_ptr += output_width;
        }
#else
        int32_t* temp_out_ptr = out_ptr;
        int index = 0;
        for (int square_row = 0; square_row < 4; square_row++) {
          for (int square_col = 0; square_col < 4; square_col++) {
            temp_out_ptr[square_col] = max_indices[index];
            index++;
          }
          temp_out_ptr += output_width;
        }
#endif
      }  // end regular bilinear resize

      in_ptr += input_argmax_depth;
      out_ptr += 4;
    }  // for col
    out_ptr += 3 * output_width;
  }  // for row
}

}  // namespace

TfLiteStatus ResizeArgmax_Prepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 3);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  const TfLiteTensor* input;
  TF_LITE_ENSURE_OK(context, GetInputSafe(context, node, kInputTensor, &input));
  const TfLiteTensor* size;
  TF_LITE_ENSURE_OK(context, GetInputSafe(context, node, kSizeTensor, &size));
  const TfLiteTensor* axis;
  TF_LITE_ENSURE_OK(context, GetInputSafe(context, node, kAxis, &axis));
  TF_LITE_ENSURE_EQ(context, *axis->data.i32, input->dims->size - 1);
  TfLiteTensor* output;
  TF_LITE_ENSURE_OK(context,
                    GetOutputSafe(context, node, kOutputTensor, &output));

  return kTfLiteOk;
}

struct ResizeArgmaxTask : cpu_backend_threadpool::Task {
  ResizeArgmaxTask(uint8_t* input, int input_width, int input_height,
                   int input_argmax_depth, int32_t* output, int output_width,
                   int output_height, std::atomic<int>& start_row,
                   int row_count, int move, tflite::Profiler* profiler)
      : input_(input),
        input_width_(input_width),
        input_height_(input_height),
        input_argmax_depth_(input_argmax_depth),
        output_(output),
        output_width_(output_width),
        output_height_(output_height),
        start_row_(start_row),
        row_count_(row_count),
        move_(move),
        profiler_(profiler) {}

  void Run() override {
    int start_row;
    while ((start_row = start_row_ += move_) - move_ < row_count_) {
      resize_argmax_task_4x(input_, input_width_, input_height_,
                            input_argmax_depth_, output_, output_width_,
                            output_height_, start_row - move_, move_, move_,
                            profiler_);
    }
  }

  uint8_t* input_;
  int input_width_;
  int input_height_;
  int input_argmax_depth_;
  int32_t* output_;
  int output_width_;
  int output_height_;
  std::atomic<int>& start_row_;
  int row_count_;
  int move_;
  tflite::Profiler* profiler_;
};

void TaskFunc(ResizeArgmaxTask* task) { task->Run(); }

TfLiteStatus ResizeArgmax_Invoke(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteTensor* input;
  TF_LITE_ENSURE_OK(context, GetInputSafe(context, node, kInputTensor, &input));
  TfLiteTensor* output;
  TF_LITE_ENSURE_OK(context,
                    GetOutputSafe(context, node, kOutputTensor, &output));
  int input_width = input->dims->data[1];
  int input_height = input->dims->data[2];
  int input_argmax_depth = input->dims->data[3];

  int output_width = output->dims->data[1];
  int output_height = output->dims->data[2];
  tflite::CpuBackendContext* cpu_backend_context =
      tflite::CpuBackendContext::GetFromContext(context);
  const int thread_count = std::min(2, cpu_backend_context->max_num_threads());

  int move = (thread_count == 1) ? 1 : 16;
  uint8_t* argmax_input_ptr = input->data.uint8;
  int32_t* argmax_output_ptr = output->data.i32;
  for (int row = 0; row < input_height; row += move) {
    for (int col = 0; col < input_width; col++) {
      *argmax_output_ptr = ArgMaxVector(argmax_input_ptr, input_argmax_depth);
      argmax_input_ptr += input_argmax_depth;
      argmax_output_ptr += 4;
    }
    argmax_input_ptr += input_argmax_depth * input_width * (move - 1);
    argmax_output_ptr += output_width * (4 * move - 1);
  }
  if (thread_count == 1) {
    resize_argmax_task_4x(input->data.uint8, input_width, input_height,
                          input_argmax_depth, output->data.i32, output_width,
                          output_height, 0, input_height, 1,
                          (tflite::Profiler*)context->profiler);
  } else {
    std::vector<ResizeArgmaxTask> tasks;
    tasks.reserve(thread_count);
    std::atomic<int> start_row;
    start_row = 0;
    for (int i = 0; i < thread_count; i++) {
      tasks.emplace_back(input->data.uint8, input_width, input_height,
                         input_argmax_depth, output->data.i32, output_width,
                         output_height, start_row, input->dims->data[2], move,
                         (tflite::Profiler*)context->profiler);
    }
    cpu_backend_threadpool::Execute(tasks.size(), tasks.data(),
                                    cpu_backend_context);
    std::vector<std::thread> task_thread(thread_count);
    for (int i = 0; i < thread_count; ++i) {
      task_thread[i] = std::thread(TaskFunc, &tasks[i]);
    }
    for (int i = 0; i < thread_count; ++i) {
      task_thread[i].join();
    }
  }

  return kTfLiteOk;
}

TfLiteRegistration* Register_ResizeArgmax() {
  static TfLiteRegistration r = {nullptr, nullptr, ResizeArgmax_Prepare,
                                 ResizeArgmax_Invoke};
  return &r;
}
