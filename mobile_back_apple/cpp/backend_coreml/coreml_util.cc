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
                                          encoding:[NSString defaultCStringEncoding]]];
    }
    _featureNames = [NSSet setWithArray:names];
  }
  return _featureNames;
}

- (MLFeatureValue *)featureValueForName:(NSString *)featureName {
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
      MLMultiArray *mlArray = [[MLMultiArray alloc] initWithDataPointer:input.data
                                                                  shape:shape
                                                               dataType:MLMultiArrayDataTypeFloat32
                                                                strides:strides
                                                            deallocator:(^(void *bytes){})error:&error];
      if (error != nil) {
        NSLog(@"Failed to create MLMultiArray for feature %@ error: %@", featureName,
              [error localizedDescription]);
        return nil;
      }
      auto *mlFeatureValue = [MLFeatureValue featureValueWithMultiArray:mlArray];
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
  MultiArrayFeatureProvider *inputFeatures;
  MultiArrayFeatureProvider *outputFeatures;
}

- (nullable instancetype)initWithModelPath:(const char *)modelPath {
  self = [super init];
  if (self) {
    try {
      NSError* error = nil;
      modelURL = [NSURL URLWithString: [NSString stringWithCString: modelPath encoding: NSUTF8StringEncoding]];
      NSURL *compiledModelURL = [MLModel compileModelAtURL: modelURL error: &error];
      if (error != nil) {
        NSLog(@"compileModelAtURL failed: %@", [error localizedDescription]);
        return nil;
      }
      MLModelConfiguration* config = [MLModelConfiguration alloc];
      config.computeUnits = MLComputeUnitsAll;
      mlmodel = [MLModel modelWithContentsOfURL: compiledModelURL
                                  configuration: config
                                          error: &error];
      if (error != NULL) {
        NSLog(@"modelWithContentsOfURL failed: %@", [error localizedDescription]);
        return nil;
      }
      NSLog(@"modelDescription: %@", [mlmodel modelDescription]);
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

- (bool)setInput:(void*) data {
  
  NSArray<NSNumber *> *inputShape;
  MLMultiArrayDataType inputDataType;
  MLMultiArrayShapeConstraint *inputShapeConstraint;
  NSString *inputName;
  
  NSDictionary *inputDict = [[mlmodel modelDescription] inputDescriptionsByName];
  for (NSString *key in [inputDict allKeys]) {
    NSLog(@"key: %@", key);
    inputName = [inputDict[key] name];
    NSLog(@"value name: %@", [inputDict[key] name]);
    NSLog(@"value type: %ld", ((MLFeatureDescription *)inputDict[key]).type);
    if (((MLFeatureDescription *)inputDict[key]).type == MLFeatureTypeMultiArray) {
      NSLog(@"constraint: %@", [((MLFeatureDescription *)inputDict[key]) multiArrayConstraint]);
      inputShape = [[((MLFeatureDescription *)inputDict[key]) multiArrayConstraint] shape];
      inputDataType = [[((MLFeatureDescription *)inputDict[key]) multiArrayConstraint] dataType];
      inputShapeConstraint = [[((MLFeatureDescription *)inputDict[key]) multiArrayConstraint] shapeConstraint];
    }
  }
  std::vector<int> shape;
  for (int i = 0; i < [inputShape count]; i++) {
    shape.push_back([inputShape[i] intValue]);
  }
  // TODO(anhappdev): currently mlperf_backend_get_input_type and mlperf_backend_get_output_type always returns Float32
  // convert float32 array to an input feature
  TensorData inputTensorData = {
    (float *) data,
    [inputName cStringUsingEncoding:[NSString defaultCStringEncoding]],
    shape
  };
  std::vector<TensorData> inputVector;
  inputVector.push_back(inputTensorData);
  
  inputFeatures = [[MultiArrayFeatureProvider alloc]
                   initWithInputs:(const std::vector<TensorData> *)&inputVector];
  
  NSError *error;
  outputFeatures = (MultiArrayFeatureProvider *)[mlmodel predictionFromFeatures:inputFeatures
                                                                          error:&error];
  if (error) {
    NSLog(@"Failed to predict with %@, error: %@", inputFeatures, [error localizedDescription]);
    return false;
  }
  NSLog(@"output names %@", [outputFeatures featureNames]);
  // TODO(anhappdev): input name is hard-coded for image classification
  NSLog(@"output Softmax: %@", [outputFeatures featureValueForName:@"Softmax"]);
  
  MLMultiArray *outputArray = [[outputFeatures featureValueForName:@"Softmax"] multiArrayValue];

  // get float array from output feature
  __block float *foo;
  // TODO(anhappdev): Error: No visible @interface for 'MLMultiArray' declares the selector 'getBytesWithHandler:'
//  [outputArray getBytesWithHandler:(^(const void *bytes, NSInteger size) {
//    NSLog(@"buffer size = %ld", size);
//    foo = (float *)bytes;
//  })];
//
//  // check if the output meet our expectation
//  float max = 0;
//  int index = 0;
//  for (int i = 0; i < 1001; i++) {
//    if (foo[i] > max) {
//      max = foo[i];
//      index = i;
//    }
//  }
//  NSLog(@"%d\n", index);
  
  return true;
}

- (bool)issueQueries {
  return false;
}

- (bool)flushQueries {
  return false;
}

- (bool)getOutput:(void *_Nonnull*_Nonnull) data {
  return false;
}


@end

