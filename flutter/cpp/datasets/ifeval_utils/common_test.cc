/* Copyright 2026 The MLPerf Authors. All Rights Reserved.

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

#include "flutter/cpp/datasets/ifeval_utils/common.h"

#include <string>

#include "gtest/gtest.h"

namespace mlperf {
namespace mobile {
namespace ifeval {
namespace {

// A response that opens with a markdown modifier used to read s[i - 1] with
// i == 0, i.e. index SIZE_MAX. Under -O2 that crashed the accuracy pass with
// EXC_BAD_ACCESS at 0xffffffffffffffff.
TEST(RemoveFontModifiers, StripsALeadingModifier) {
  EXPECT_EQ(remove_font_modifiers("*Hello* world"), "Hello world");
  EXPECT_EQ(remove_font_modifiers("_x_"), "x");
  EXPECT_EQ(remove_font_modifiers("~y~"), "y");
  EXPECT_EQ(remove_font_modifiers("\\z"), "z");
}

TEST(RemoveFontModifiers, KeepsAnEscapedModifier) {
  EXPECT_EQ(remove_font_modifiers("a\\*b"), "a*b");
}

TEST(RemoveFontModifiers, HandlesTheRemainingMarkup) {
  EXPECT_EQ(remove_font_modifiers(""), "");
  EXPECT_EQ(remove_font_modifiers("`code`"), "code");
  EXPECT_EQ(remove_font_modifiers("# Title"), " Title");
  EXPECT_EQ(remove_font_modifiers("> quote"), " quote");
}

TEST(TransformResponse, AppliesTheFontMask) {
  EXPECT_EQ(transform_response("*starts with emphasis*")[1],
            "starts with emphasis");
}

// ends_with looks at the last suf.size() + threshold characters. When the
// response is shorter than that window the subtraction used to wrap, and
// substr threw std::out_of_range.
TEST(EndsWith, AcceptsAResponseShorterThanTheWindow) {
  EXPECT_TRUE(ends_with("the end.", "the end.", 3));
  EXPECT_TRUE(ends_with("...and the end.", "the end.", 3));
}

TEST(EndsWith, StillRejectsNonMatches) {
  EXPECT_FALSE(ends_with("something else", "the end.", 3));
  EXPECT_FALSE(ends_with("short", "a much longer suffix", 3));
}

TEST(EndsWith, MatchesExactlyAtThresholdZero) {
  EXPECT_TRUE(ends_with("abc", "abc", 0));
  EXPECT_FALSE(ends_with("abcd", "abc", 0));
}

}  // namespace
}  // namespace ifeval
}  // namespace mobile
}  // namespace mlperf

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
