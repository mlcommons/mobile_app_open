/* Copyright 2019 The MLPerf Authors. All Rights Reserved.

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
#include "utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace mlperf {
namespace mobile {
namespace {

using ::testing::ElementsAreArray;

TEST(GetTopK, Top1) {
  std::vector<int> values{5, 3, 6, 8};
  std::vector<int> output = GetTopK(values.data(), values.size(), 1, 0);
  EXPECT_THAT(output, ElementsAreArray({3}));
}

TEST(GetTopK, Top3) {
  std::vector<int> values{5, 3, 6, 8};
  std::vector<int> output = GetTopK(values.data(), values.size(), 3, 0);
  EXPECT_THAT(output, ElementsAreArray({3, 2, 0}));
}

TEST(GetTopK, Offset1) {
  std::vector<int> values{5, 3, 6, 8};
  std::vector<int> output = GetTopK(values.data(), values.size(), 3, 1);
  EXPECT_THAT(output, ElementsAreArray({2, 1, 0}));
}

TEST(GetTopK, BiggerK) {
  std::vector<int> values{5, 3, 6, 8};
  std::vector<int> output = GetTopK(values.data(), values.size(), 4, 1);
  EXPECT_THAT(output, ElementsAreArray({2, 1, 0, 0}));
}

TEST(GetTopK, BiggerOffset) {
  std::vector<int> values{5, 3, 6, 8};
  ASSERT_THROW(GetTopK(values.data(), values.size(), 4, 5), std::bad_alloc);
}

}  // namespace
}  // namespace mobile
}  // namespace mlperf

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
