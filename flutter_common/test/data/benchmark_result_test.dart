import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench_common/data/results/benchmark_result.dart';

void main() {
  group('Accuracy', () {
    test('comparable', () {
      const accuracySmall = Accuracy(normalized: 0.6, formatted: '0.6 mAP');
      const accuracyBig = Accuracy(normalized: 0.8, formatted: '0.8 mAP');
      expect(accuracyBig, greaterThan(accuracySmall));
      expect(accuracySmall, lessThan(accuracyBig));
    });
    test('equatable', () {
      const accuracy1 = Accuracy(normalized: 0.8, formatted: '0.8 mAP');
      const accuracy2 = Accuracy(normalized: 0.80, formatted: '0.80 mAP');
      const accuracy3 = Accuracy(normalized: 0.801, formatted: '0.801 mAP');
      expect(accuracy1, equals(accuracy2));
      expect(accuracy1, isNot(equals(accuracy3)));
    });
  });

  group('Throughput', () {
    test('comparable', () {
      const throughputSmall = Throughput(value: 30.45);
      const throughputBig = Throughput(value: 50.12);
      expect(throughputBig, greaterThan(throughputSmall));
      expect(throughputSmall, lessThan(throughputBig));
      expect(throughputSmall, isNot(equals(throughputBig)));
    });
    test('equatable', () {
      const throughput1 = Throughput(value: 45.123);
      const throughput2 = Throughput(value: 45.123);
      const throughput3 = Throughput(value: 45);
      expect(throughput1, equals(throughput2));
      expect(throughput1, isNot(equals(throughput3)));
    });
  });
}
