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

using namespace std;

struct MLFeature {
  void *data;
  MLFeatureDescription *description;
};

@interface MLMultiArrayFeatureProvider : NSObject <MLFeatureProvider> {
  const std::vector<MLFeature> *_inputs;
  NSDictionary<NSString *, MLFeatureValue *> *featureValues;
}
@end

@implementation MLMultiArrayFeatureProvider

@synthesize featureNames;

- (instancetype)initWithInputs:(const std::vector<MLFeature> *)inputs {
  self = [super init];
  _inputs = inputs;
  auto values = [[NSMutableDictionary alloc] init];
  NSMutableArray *names = [[NSMutableArray alloc] init];
  for (auto &input : *_inputs) {
    NSString *featureName = input.description.name;
    if (featureName.length == 0) {
      return nil;
    }
    [names addObject:featureName];

    auto constraint = input.description.multiArrayConstraint;
    NSArray<NSNumber *> *shape = constraint.shape;
    NSMutableArray<NSNumber *> *strides = [[NSMutableArray alloc] init];
    for (int i = 0; i < shape.count; i++) {
      [strides addObject:@1];
    }
    for (NSUInteger i = shape.count - 1; i > 0; i--) {
      int stride = strides[i].intValue * shape[i].intValue;
      strides[i - 1] = @(stride);
    }
    NSError *error = nil;
    MLMultiArray *mlArray =
        [[MLMultiArray alloc] initWithDataPointer:input.data
                                            shape:constraint.shape
                                         dataType:constraint.dataType
                                          strides:strides
                                      deallocator:nil
                                            error:&error];
    if (error != nil) {
      NSLog(@"Failed to create MLMultiArray for feature %@ with error: %@",
            featureName, error);
      return nil;
    }
    auto *mlFeatureValue = [MLFeatureValue featureValueWithMultiArray:mlArray];
    values[featureName] = mlFeatureValue;
  }
  featureNames = [NSSet setWithArray:names];
  featureValues = [[NSDictionary alloc] initWithDictionary:values];
  return self;
}

- (MLFeatureValue *)featureValueForName:(NSString *)featureName {
  auto value = featureValues[featureName];
  if (value == nil) {
    NSLog(@"Feature %@ not found", featureName);
  }
  return value;
}
@end

@interface MLMultiArrayBatchProvider : NSObject <MLBatchProvider> {
  const vector<vector<MLFeature>> *_inputs;
}
@end

@implementation MLMultiArrayBatchProvider

@synthesize count;

- (instancetype)initWithInputs:(const vector<vector<MLFeature>> *)inputs {
  self = [super init];
  _inputs = inputs;
  count = _inputs->size();
  return self;
}

- (nonnull id<MLFeatureProvider>)featuresAtIndex:(NSInteger)index {
  auto inputFeature =
      [[MLMultiArrayFeatureProvider alloc] initWithInputs:&_inputs->at(index)];
  return inputFeature;
}

@end

@implementation CoreMLExecutor {
 @protected
  NSURL *modelURL;
  MLModel *mlmodel;
  uint batchSize;
  uint inputCount;
  uint outputCount;
  vector<vector<MLFeature>> inputFeatures;
  vector<vector<MLFeature>> outputFeatures;
  NSArray<NSString *> *inputNames;
  NSArray<NSString *> *outputNames;
  id<MLBatchProvider> outputProvider;
}

- (nullable instancetype)initWithModelPath:(const char *)modelPath
                                 batchSize:(int)batchSize {
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
      auto _inputNames =
          [[[mlmodel modelDescription] inputDescriptionsByName] allKeys];
      auto _outputNames =
          [[[mlmodel modelDescription] outputDescriptionsByName] allKeys];
      inputNames = [_inputNames
          sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
      outputNames = [_outputNames
          sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
      self->batchSize = batchSize > 1 ? batchSize : 1;
      inputCount = (uint)inputNames.count;
      outputCount = (uint)outputNames.count;
      [self prepareFeatureVector];
      NSLog(@"inputNames: %@", inputNames);
      NSLog(@"outputNames: %@", outputNames);
      NSLog(@"batchSize: %d", batchSize);
    } catch (const std::exception &exception) {
      NSLog(@"%s", exception.what());
      return nil;
    }
  }
  NSLog(@"[CoreMLExecutor init]");
  return self;
}

- (void)prepareFeatureVector {
  vector<vector<MLFeature>> _inputData(batchSize,
                                       vector<MLFeature>(inputCount));
  inputFeatures = _inputData;
  vector<vector<MLFeature>> _outputData(batchSize,
                                        vector<MLFeature>(outputCount));
  outputFeatures = _outputData;
}

- (void)clearFeatureVector {
  inputFeatures.clear();
  outputFeatures.clear();
}

- (void)dealloc {
  [self clearFeatureVector];
  NSLog(@"[CoreMLExecutor dealloc]");
}

// MARK: Input

- (int)getInputCount {
  return (int)inputCount;
}

- (MLFeatureDescription *)getInputAt:(int)i {
  NSString *name = inputNames[i];
  auto inputs = [[mlmodel modelDescription] inputDescriptionsByName];
  auto input = inputs[name];
  return input;
}

- (int)getInputSizeAt:(int)i {
  auto input = [self getInputAt:i];
  int inputSize = 1;
  for (NSNumber *n in [[input multiArrayConstraint] shape]) {
    inputSize *= n.intValue;
  }
  assert(inputSize > 0);
  return inputSize;
}

- (MLMultiArrayDataType)getInputTypeAt:(int)i {
  auto input = [self getInputAt:i];
  auto type = [[input multiArrayConstraint] dataType];
  return type;
}

// MARK: Output

- (int)getOutputCount {
  return (int)outputCount;
}

- (MLFeatureDescription *)getOutputAt:(int)i {
  NSString *name = outputNames[i];
  auto outputs = [[mlmodel modelDescription] outputDescriptionsByName];
  auto output = outputs[name];
  return output;
}

- (int)getOutputSizeAt:(int)i {
  auto output = [self getOutputAt:i];
  int outputSize = 1;
  for (NSNumber *n in [[output multiArrayConstraint] shape]) {
    outputSize *= n.intValue;
  }
  assert(outputSize > 0);
  return outputSize;
}

- (MLMultiArrayDataType)getOutputTypeAt:(int)i {
  auto output = [self getOutputAt:i];
  auto type = [[output multiArrayConstraint] dataType];
  return type;
}

// MARK: Inference

- (bool)setInputData:(void *)data at:(int)i batchIndex:(int)batchIndex {
  auto input = [self getInputAt:i];
  MLFeature inputFeatureData = {data, input};
  inputFeatures.at(batchIndex).at(i) = inputFeatureData;
  return true;
}

- (bool)issueQueries {
  @autoreleasepool {
    NSError *error = nil;
    auto batch =
        [[MLMultiArrayBatchProvider alloc] initWithInputs:&inputFeatures];
    outputProvider = [mlmodel predictionsFromBatch:batch error:&error];
    if (error != nil) {
      NSLog(@"Failed to predict with error: %@", error);
      return false;
    }
    for (uint batchIndex = 0; batchIndex < batchSize; batchIndex++) {
      id<MLFeatureProvider> outputFeature =
          [outputProvider featuresAtIndex:batchIndex];
      for (uint outputIndex = 0; outputIndex < outputCount; outputIndex++) {
        NSString *name = outputNames[outputIndex];
        MLFeatureValue *outputValue = [outputFeature featureValueForName:name];
        MLMultiArray *outputArray = [outputValue multiArrayValue];
        void *data = outputArray.dataPointer;
        if (data == nullptr) {
          return false;
        }
        MLFeature outputFeatureData = {data, nil};
        outputFeatures.at(batchIndex).at(outputIndex) = outputFeatureData;
      }
    }
    return true;
  }
}

- (bool)flushQueries {
  [self clearFeatureVector];
  [self prepareFeatureVector];
  return true;
}

- (bool)getOutputData:(void **)data at:(int)i batchIndex:(int)batchIndex {
  auto outputFeatureData = outputFeatures.at(batchIndex).at(i);
  *data = outputFeatureData.data;
  return true;
}

@end
