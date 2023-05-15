#ifndef MOBILE_APP_OPEN_BACKEND_COREML_UTIL_H
#define MOBILE_APP_OPEN_BACKEND_COREML_UTIL_H

#import <CoreML/CoreML.h>
#import "Foundation/Foundation.h"

@interface CoreMLExecutor : NSObject

- (nullable instancetype)initWithModelPath:(const char *_Nonnull)modelPath batchSize:(int)batchSize;

- (int)getInputCount;
- (int)getInputSizeAt:(int)i;
- (NSNumber * _Nullable)getInputTypeAt:(NSInteger)i;

- (int)getOutputCount;
- (int)getOutputSizeAt:(int)i;
- (NSNumber * _Nullable)getOutputTypeAt:(NSInteger)i;

- (bool)issueQueries;
- (bool)flushQueries;

- (bool)setInputData:(void *_Nonnull)data at:(int)i batchIndex:(int)batchIndex;
- (bool)getOutputData:(void *_Nonnull *_Nonnull)data at:(int)i batchIndex:(int)batchIndex;

@end

#endif  // MOBILE_APP_OPEN_BACKEND_COREML_UTIL_H
