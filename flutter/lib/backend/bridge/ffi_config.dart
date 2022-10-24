import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'package:mlperfbench/protos/mlperf_task.pb.dart' as pb;
import 'handle.dart';

class _RunOut extends Struct {
  @Int32()
  external int size;

  external Pointer<Uint8> data;
}

const _runName = 'dart_ffi_mlperf_config';
const _freeName = 'dart_ffi_mlperf_config_free';

typedef _Run = Pointer<_RunOut> Function(Pointer<Utf8> pbContent);
final _run = getBridgeHandle().lookupFunction<_Run, _Run>(_runName);

typedef _Free1 = Void Function(Pointer<_RunOut>);
typedef _Free2 = void Function(Pointer<_RunOut>);
final _free = getBridgeHandle().lookupFunction<_Free1, _Free2>(_freeName);

pb.MLPerfConfig getMLPerfConfig(String pbtxtContent) {
  final pbtxtContentUtf8 = pbtxtContent.toNativeUtf8();
  final runOut = _run(pbtxtContentUtf8);
  malloc.free(pbtxtContentUtf8);

  if (runOut.address == 0) {
    throw '$_runName result: nullprt';
  }

  try {
    if (runOut.ref.data.address == 0) {
      throw '$_runName result: data: nullptr';
    }
    final view = runOut.ref.data.asTypedList(runOut.ref.size);
    return pb.MLPerfConfig.fromBuffer(view);
  } finally {
    _free(runOut);
  }
}
