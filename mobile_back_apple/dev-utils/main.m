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

#import <Foundation/Foundation.h>
#import <mobile_back_apple-Swift.h>
#import "mobile_back_apple-Bridging-Header.h"


void testIC() {
    const char * modelPath = "/Volumes/work/Programming/ScopicSoftware/MLCommons/mobile/mobile_back_apple/dev-resources/mobilenet_edgetpu/MobilenetEdgeTPU.mlmodel";
    const char * inputFile = "/Volumes/work/Programming/ScopicSoftware/MLCommons/mobile/mobile_back_apple/dev-resources/imagenet/grace_hopper.raw";
    const char *acceleratorName = "cpu&gpu";
    const int expectedPredictedIndex = 653; // expected index for grace_hopper is 653
    
    int batchSize = 32;
    NSError *error = nil;
    CoreMLExecutor *coreMLExecutor = [[CoreMLExecutor alloc] initWithModelPath:modelPath batchSize:batchSize acceleratorName:acceleratorName error: &error];
    assert(error == nil);
    [coreMLExecutor getInputCount];
    [coreMLExecutor getOutputCount];
    
    NSNumber *inputTypeNumber = [coreMLExecutor getInputTypeAt:0];
    MLMultiArrayDataType inputType = (MLMultiArrayDataType)inputTypeNumber.integerValue;
    assert(inputType == MLMultiArrayDataTypeFloat32);
    
    NSNumber *outputTypeNumber = [coreMLExecutor getOutputTypeAt:0];
    MLMultiArrayDataType outputType = (MLMultiArrayDataType)outputTypeNumber.integerValue;
    assert(outputType == MLMultiArrayDataTypeFloat32);
    
    int fd = open(inputFile, O_RDONLY);
    int inputSize = [coreMLExecutor getInputSizeAt:0];
    assert(inputSize == 150528);
    unsigned char *inputData_uint8 = (unsigned char *)malloc(inputSize);
    ssize_t r_size = read(fd, inputData_uint8, inputSize);
    assert(inputSize == r_size);
    
    int outputSize = [coreMLExecutor getOutputSizeAt:0];;
    assert(outputSize == 1001);
    
    int queryCount = 5;
    for (int q = 0; q < queryCount; q++) {
        // convert uint8 values to float32 values and normalize
        float *inputData_float = (float *)malloc(inputSize * sizeof(float));
        for (int i = 0; i < inputSize; i++) {
            inputData_float[i] = (float)((inputData_uint8[i] * 1.0 - 128) / 255.0);
        }
        for (int i = 0; i < batchSize; i++) {
            [coreMLExecutor setInputData:inputData_float at:0 batchIndex:i];
        }
        
        [coreMLExecutor issueQueries];
        
        float *outputData = (float *)malloc(outputSize * sizeof(float));
        [coreMLExecutor getOutputData:(void **)&outputData at:0 batchIndex:0];
        
        int predictedIndex = 0;
        float predictedScore = 0;
        for (int i = 0; i < outputSize; i++) {
            if (outputData[i] > predictedScore) {
                predictedScore = outputData[i];
                predictedIndex = i;
            }
            assert(outputData[i] >= 0);
            assert(outputData[i] <= 1);
        }
        NSLog(@"predictedIndex: %d", predictedIndex);
        NSLog(@"predictedScore: %f", predictedScore);
        assert(predictedIndex == expectedPredictedIndex);
        
        [coreMLExecutor flushQueries];
        
    }
    
    coreMLExecutor = nil;
}

void testOD() {
    const char * modelPath = "/Volumes/work/Programming/ScopicSoftware/MLCommons/mobile/mobile_back_apple/dev-resources/mobiledet/MobileDet.mlmodel";
    const char *acceleratorName = "cpu&gpu";
    NSError *error = nil;
    CoreMLExecutor *coreMLExecutor = [[CoreMLExecutor alloc] initWithModelPath:modelPath batchSize:1 acceleratorName:acceleratorName error: &error];
    assert(error == nil);
    [coreMLExecutor getInputCount];
    [coreMLExecutor getOutputCount];
    
    int inputSize = [coreMLExecutor getInputSizeAt:0];
    assert(inputSize == 307200);
    
    float *inputData_float = (float *)malloc(inputSize * sizeof(float));
    for (int i = 0; i < inputSize; i++) {
        inputData_float[i] = (float)(0.5);
    }
    [coreMLExecutor setInputData:inputData_float at:0 batchIndex:0];
    
    [coreMLExecutor issueQueries];
    int outputSize_0 = [coreMLExecutor getOutputSizeAt:0];
    assert(outputSize_0 == 40);
    int outputSize_1 = [coreMLExecutor getOutputSizeAt:1];
    assert(outputSize_1 == 10);
    int outputSize_2 = [coreMLExecutor getOutputSizeAt:2];
    assert(outputSize_2 == 10);
    int outputSize_3 = [coreMLExecutor getOutputSizeAt:3];
    assert(outputSize_3 == 1);
    
    float *outputData = (float *)malloc(outputSize_0 * sizeof(float));
    [coreMLExecutor getOutputData:(void **)&outputData at:0 batchIndex:0];
    
    [coreMLExecutor flushQueries];
    coreMLExecutor = nil;
}

void testIS() {
    
}

void testLU() {
    const char * modelPath = "/Volumes/work/Programming/ScopicSoftware/MLCommons/mobile/mobile_back_apple/dev-resources/mobilebert/MobileBERT.mlmodel";
    const char *acceleratorName = "cpu&gpu";
    NSError *error = nil;
    CoreMLExecutor *coreMLExecutor = [[CoreMLExecutor alloc] initWithModelPath:modelPath batchSize:1 acceleratorName:acceleratorName error: &error];
    assert(error == nil);
    [coreMLExecutor getInputCount];
    [coreMLExecutor getOutputCount];
    
    NSNumber *inputTypeNumber = [coreMLExecutor getInputTypeAt:0];
    MLMultiArrayDataType inputType = (MLMultiArrayDataType)inputTypeNumber.integerValue;
    assert(inputType == MLMultiArrayDataTypeInt32);
    
    NSNumber *outputTypeNumber = [coreMLExecutor getOutputTypeAt:0];
    MLMultiArrayDataType outputType = (MLMultiArrayDataType)outputTypeNumber.integerValue;
    assert(outputType == MLMultiArrayDataTypeFloat32);
    
    int inputSize = [coreMLExecutor getInputSizeAt:0];
    assert(inputSize == 1*384);
    
    // convert uint8 values to float32 values and normalize
    int32_t *inputData = (int32_t *)malloc(inputSize * sizeof(float));
    for (int i = 0; i < inputSize; i++) {
        inputData[i] = (int32_t)(1);
    }
    [coreMLExecutor setInputData:inputData at:0 batchIndex:0];
    [coreMLExecutor setInputData:inputData at:1 batchIndex:0];
    [coreMLExecutor setInputData:inputData at:2 batchIndex:0];
    
    [coreMLExecutor issueQueries];
    int outputSize_0 = [coreMLExecutor getOutputSizeAt:0];
    assert(outputSize_0 == 1*384);
    int outputSize_1 = [coreMLExecutor getOutputSizeAt:1];
    assert(outputSize_1 == 1*384);
    
    float *outputData = (float *)malloc(outputSize_0 * sizeof(float));
    [coreMLExecutor getOutputData:(void **)&outputData at:0 batchIndex:0];
    
    
    [coreMLExecutor flushQueries];
    coreMLExecutor = nil;
}

int main(int argc, const char * argv[]) {
    
    NSString *line = @"========================================================================";
    NSLog(@"START");
    testIC();
    NSLog(@"%@", line);
    testOD();
    NSLog(@"%@", line);
    testLU();
    NSLog(@"END");
    
    return 0;
}
