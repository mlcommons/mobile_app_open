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

#import <CoreML/CoreML.h>
#import <Foundation/Foundation.h>

#include <string>
#include <vector>

struct TensorData {
  float *data;
  const std::string name;
  std::vector<int> shape;
};

@interface MultiArrayFeatureProvider : NSObject <MLFeatureProvider> {
  const std::vector<TensorData> *_inputs;
  NSSet *_featureNames;
}
@end

@implementation MultiArrayFeatureProvider

- (instancetype)initWithInputs:(const std::vector<TensorData> *)inputs {
  self = [super init];
  _inputs = inputs;
  for (auto &input : *_inputs) {
    if (input.name.empty()) {
      return nil;
    }
  }
  return self;
}

- (NSSet<NSString *> *)featureNames {
  if (_featureNames == nil) {
    NSMutableArray *names = [[NSMutableArray alloc] init];
    for (auto &input : *_inputs) {
      [names addObject:[NSString stringWithCString:input.name.c_str()
                                          encoding:NSUTF8StringEncoding]];
    }
    _featureNames = [NSSet setWithArray:names];
  }
  return _featureNames;
}

- (MLFeatureValue *)featureValueForName:(NSString *)featureName {
  NSLog(@"featureValueForName: %@", featureName);
  for (auto &input : *_inputs) {
    if ([featureName cStringUsingEncoding:NSUTF8StringEncoding] == input.name) {
      NSArray *shape = @[
        @(input.shape[0]),
        @(input.shape[1]),
        @(input.shape[2]),
        @(input.shape[3]),
      ];

      NSArray *strides = @[
        @(input.shape[1] * input.shape[2] * input.shape[3]),
        @(input.shape[2] * input.shape[3]),
        @(input.shape[3]),
        @1,
      ];

      NSError *error = nil;
      MLMultiArray *mlArray =
          [[MLMultiArray alloc] initWithDataPointer:input.data
                                              shape:shape
                                           dataType:MLMultiArrayDataTypeFloat32
                                            strides:strides
                                        deallocator:(^(void *bytes){
                                                    })error:&error];
      if (error != nil) {
        NSLog(@"Failed to create MLMultiArray for feature %@ error: %@",
              featureName, [error localizedDescription]);
        return nil;
      }
      auto *mlFeatureValue =
          [MLFeatureValue featureValueWithMultiArray:mlArray];
      return mlFeatureValue;
    }
  }

  NSLog(@"Feature %@ not found", featureName);
  return nil;
}
@end

@implementation CoreMLExecutor {
 @protected
  NSURL *modelURL;
  MLModel *mlmodel;
  std::vector<TensorData> inputData;
  NSArray<NSString *> *inputNames;
  MultiArrayFeatureProvider *inputFeature;
  NSArray<NSString *> *outputNames;
  id<MLFeatureProvider> outputFeature;
}

- (nullable instancetype)initWithModelPath:(const char *)modelPath {
  self = [super init];
  if (self) {
    try {
      NSError *error = nil;
      modelURL = [NSURL
          URLWithString:[NSString stringWithCString:modelPath
                                           encoding:NSUTF8StringEncoding]];
      NSURL *compiledModelURL =
          [MLModel compileModelAtURL:modelURL error:&error];
      if (error != nil) {
        NSLog(@"compileModelAtURL failed: %@", [error localizedDescription]);
        return nil;
      }
      MLModelConfiguration *config = [MLModelConfiguration alloc];
      config.computeUnits = MLComputeUnitsAll;
      mlmodel = [MLModel modelWithContentsOfURL:compiledModelURL
                                  configuration:config
                                          error:&error];
      if (error != NULL) {
        NSLog(@"modelWithContentsOfURL failed: %@",
              [error localizedDescription]);
        return nil;
      }
      NSLog(@"modelDescription: %@", [mlmodel modelDescription]);
      inputNames =
          [[[mlmodel modelDescription] inputDescriptionsByName] allKeys];
      outputNames =
          [[[mlmodel modelDescription] outputDescriptionsByName] allKeys];
    } catch (const std::exception &exception) {
      NSLog(@"%s", exception.what());
      return nil;
    }
  }
  return self;
}

- (void)dealloc {
  [self flushQueries];
  NSLog(@"[CoreMLExecutor dealloc]");
}

- (int)getInputCount {
  NSInteger inputCount =
      [[[mlmodel modelDescription] inputDescriptionsByName] count];
  NSLog(@"inputCount: %ld", inputCount);
  return (int)inputCount;
}

- (int)getInputSizeAt:(int)i {
  NSString *name = inputNames[i];
  auto inputs = [[mlmodel modelDescription] inputDescriptionsByName];
  auto input = [inputs objectForKey:name];
  int inputSize = 1;
  for (NSNumber *n in [[input multiArrayConstraint] shape]) {
    inputSize *= n.intValue;
  }
  NSLog(@"inputSize: %d", inputSize);
  assert(inputSize > 0);
  return int(inputSize);
}

- (int)getOutputCount {
  NSInteger outputCount =
      [[[mlmodel modelDescription] outputDescriptionsByName] count];
  NSLog(@"outputCount: %ld", outputCount);
  return (int)outputCount;
}

- (int)getOutputSizeAt:(int)i {
  NSString *name = outputNames[i];
  auto outputs = [[mlmodel modelDescription] outputDescriptionsByName];
  auto output = [outputs objectForKey:name];
  int outputSize = 1;
  for (NSNumber *n in [[output multiArrayConstraint] shape]) {
    outputSize *= n.intValue;
  }
  NSLog(@"outputSize: %d", outputSize);
  assert(outputSize > 0);
  return int(outputSize);
}

- (bool)setInput:(void *)data at:(int)i {
  NSString *name = inputNames[i];
  MLFeatureDescription *description =
      [[[mlmodel modelDescription] inputDescriptionsByName] valueForKey:name];
  NSArray<NSNumber *> *shape = [[description multiArrayConstraint] shape];

  // convert float32 array to an input feature
  std::vector<int> tensorShape;
  for (int i = 0; i < [shape count]; i++) {
    tensorShape.push_back([shape[i] intValue]);
  }
  TensorData inputTensorData = {
      (float *)data, [name cStringUsingEncoding:NSUTF8StringEncoding],
      tensorShape};
  inputData.push_back(inputTensorData);

  inputFeature = [[MultiArrayFeatureProvider alloc]
      initWithInputs:(const std::vector<TensorData> *)&inputData];

  return true;
}

- (bool)issueQueries {
  NSError *error = nil;
  outputFeature = [mlmodel predictionFromFeatures:inputFeature error:&error];
  if (error != nil) {
    NSLog(@"Failed to predict with %@, error: %@", inputFeature,
          [error localizedDescription]);
    return false;
  }
  return true;
}

- (bool)flushQueries {
  inputData.clear();
  return true;
}

- (bool)getOutput:(void **)data at:(int)i {
  NSString *name = outputNames[i];
  MLFeatureValue *outputValue = [outputFeature featureValueForName:name];
  MLMultiArray *outputArray = [outputValue multiArrayValue];
  float *outputData = (float *)outputArray.dataPointer;
  if (outputData == nullptr) {
    return false;
  }
  *data = outputData;
  return true;
}

@end
