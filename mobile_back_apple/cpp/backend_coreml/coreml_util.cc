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

#import "coreml_util.h"

#include <string>
#include <vector>

#import <CoreML/CoreML.h>
#import <Foundation/Foundation.h>


@implementation CoreMLExecutor {
@protected
  NSURL *modelURL;
  MLModel *mlmodel;
}

- (id)initWithModelPath:(const char *)modelPath {
  self = [super init];
  if (self) {
    try {
      modelURL = [NSURL URLWithString: [NSString stringWithCString: modelPath encoding: NSUTF8StringEncoding]];
      NSURL *compiledModelURL = [MLModel compileModelAtURL: modelURL error: nil];
      NSError *e;
      mlmodel = [MLModel modelWithContentsOfURL: compiledModelURL error: &e];
      if (mlmodel == nil) {
        NSLog(@"hmmm, %@", e);
      }
    } catch (const std::exception& exception) {
      NSLog(@"%s", exception.what());
      return nil;
    }
  }
  return self;
}

- (int)getInputCount {
  NSInteger inputCount = [[[mlmodel modelDescription] inputDescriptionsByName] count];
  NSLog(@"inputCount %ld", inputCount);
  return (int) inputCount;
}

- (int)getOutputCount {
  NSInteger outputCount = [[[mlmodel modelDescription] outputDescriptionsByName] count];
  NSLog(@"outputCount %ld", outputCount);
  return (int) outputCount;
}

- (void)hello {
  
  NSLog(@"Hello from CoreMLDelegate");
}

@end

