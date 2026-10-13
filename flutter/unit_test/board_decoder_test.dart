import 'package:flutter_test/flutter_test.dart';

import 'package:mlperfbench/board_decoder.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  group('BoardDecoder', () {
    test('has no boards before init', () {
      expect(BoardDecoder().boards, isEmpty);
    });

    test('init loads the bundled board database', () async {
      final decoder = BoardDecoder();
      await decoder.init();

      expect(decoder.boards, isNotEmpty);
      expect(decoder.boards.keys, everyElement(isNotEmpty));
      expect(decoder.boards.values, everyElement(isNotEmpty));
    });

    test('init is idempotent', () async {
      final decoder = BoardDecoder();
      await decoder.init();
      final firstLoad = Map<String, String>.of(decoder.boards);
      await decoder.init();

      expect(decoder.boards, firstLoad);
    });
  });
}
