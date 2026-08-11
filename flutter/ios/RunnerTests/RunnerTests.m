@import XCTest;
@import integration_test;
@import ObjectiveC.runtime;
@import UIKit;

// Custom implementation replacing INTEGRATION_TEST_IOS_RUNNER macro.
//
// The default macro runs all Dart tests inside +testInvocations (the XCTestCase
// construction phase). Xcode 15+ added a watchdog timer that kills tests still
// in construction after ~6 minutes, causing long-running integration tests to
// be terminated with "Test runner never began executing tests after launching".
// See: https://github.com/flutter/flutter/issues/145143
//
// This implementation moves test execution into an actual test method, which is
// not subject to the construction-phase watchdog. The trade-off is that all Dart
// tests are reported as a single XCTest case instead of individual cases.

@interface RunnerTests : XCTestCase
@end

@implementation RunnerTests

- (void)testIntegrationTest {
  FLTIntegrationTestRunner *integrationTestRunner = [[FLTIntegrationTestRunner alloc] init];

  __block BOOL allTestsPassed = YES;
  __block NSMutableArray<NSString *> *failures = [[NSMutableArray alloc] init];

  [integrationTestRunner testIntegrationTestWithResults:^(SEL testSelector, BOOL success, NSString *failureMessage) {
    if (!success) {
      allTestsPassed = NO;
      NSString *name = NSStringFromSelector(testSelector);
      [failures addObject:[NSString stringWithFormat:@"%@: %@", name, failureMessage ?: @"(no message)"]];
    }
  }];

  NSDictionary<NSString *, UIImage *> *capturedScreenshotsByName = integrationTestRunner.capturedScreenshotsByName;
  [capturedScreenshotsByName enumerateKeysAndObjectsUsingBlock:^(NSString *name, UIImage *screenshot, BOOL *stop) {
    XCTAttachment *attachment = [XCTAttachment attachmentWithImage:screenshot];
    attachment.lifetime = XCTAttachmentLifetimeKeepAlways;
    if (name != nil) {
      attachment.name = name;
    }
    [self addAttachment:attachment];
  }];

  if (!allTestsPassed) {
    XCTFail(@"Flutter integration test failures:\n%@", [failures componentsJoinedByString:@"\n"]);
  }
}

@end
