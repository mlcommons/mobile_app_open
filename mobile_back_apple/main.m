//
//  main.m
//  mobile_back_apple
//
//

#import <Foundation/Foundation.h>
#import "cpp/backend_coreml/coreml_util.h"

const char * modelPath = "/Users/anh/dev/mlcommons/mobile_app_open/mobile_back_apple/dev-resources/MobilenetEdgeTPU_multi_array.mlmodel";
const char * inputFile = "/Users/anh/dev/mlcommons/mobile_app_open/mobile_back_apple/dev-resources/grace_hopper.raw";
const int expectedPredictedIndex = 653; // expected index for grace_hopper is 653

int main(int argc, const char * argv[]) {
  @autoreleasepool {
    NSLog(@"START");

    CoreMLExecutor *coreMLExecutor = [[CoreMLExecutor alloc] initWithModelPath:modelPath];
    [coreMLExecutor getInputCount];
    [coreMLExecutor getOutputCount];
    
    int fd = open(inputFile, O_RDONLY);
    int inputSize = 224 * 224 * 3;
    unsigned char *inputData_uint8 = (unsigned char *)malloc(inputSize);
    ssize_t r_size = read(fd, inputData_uint8, inputSize);
    NSLog(@"size read = %zd", r_size);
    
    // convert uint8 values to float32 values and normalize
    float *inputData_float = (float *)malloc(inputSize * sizeof(float));
    for (int i = 0; i < inputSize; i++) {
      inputData_float[i] = (float)((inputData_uint8[i] * 1.0 - 128) / 255.0);
    }
    NSLog(@"%f:%f:%f\n", inputData_float[64], inputData_float[65], inputData_float[66]);
    [coreMLExecutor setInput:inputData_float];

    [coreMLExecutor issueQueries];
    int outputSize = 1001;
    float *outputData = (float *)malloc(outputSize * sizeof(float));
    [coreMLExecutor getOutput:(void **)&outputData];
    NSLog(@"outputData[1]: %f",outputData[1]);
    NSLog(@"outputData[outputSize-1]: %f",outputData[outputSize-1]);
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
    coreMLExecutor = nil;
    NSLog(@"END");
  }
  return 0;
}
