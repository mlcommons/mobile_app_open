/* Copyright 2022 The MLPerf Authors. All Rights Reserved.

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

#ifndef MOBILE_APP_OPEN_BACKEND_COREML_UTIL_H
#define MOBILE_APP_OPEN_BACKEND_COREML_UTIL_H

#import <CoreML/CoreML.h>
#import "Foundation/Foundation.h"

@interface CoreMLExecutor : NSObject

- (nullable instancetype)initWithModelPath:(const char *_Nonnull)modelPath batchSize:(int)batchSize;

- (int)getInputCount;
- (int)getInputSizeAt:(int)i;
- (MLMultiArrayDataType)getInputTypeAt:(int)i;

- (int)getOutputCount;
- (int)getOutputSizeAt:(int)i;
- (MLMultiArrayDataType)getOutputTypeAt:(int)i;

- (bool)issueQueries;
- (bool)flushQueries;

- (bool)setInputData:(void *_Nonnull)data at:(int)i batchIndex:(int)batchIndex;
- (bool)getOutputData:(void *_Nonnull *_Nonnull)data at:(int)i batchIndex:(int)batchIndex;

@end

#endif  // MOBILE_APP_OPEN_BACKEND_COREML_UTIL_H
