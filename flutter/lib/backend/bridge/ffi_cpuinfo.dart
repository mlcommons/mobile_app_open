import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'handle.dart';

class _RunOut extends Struct {
  external Pointer<Utf8> soc_name;
}

const _runName = 'dart_ffi_cpuinfo';
const _freeName = 'dart_ffi_cpuinfo_free';

typedef _Run = Pointer<_RunOut> Function();
final _run = getBridgeHandle().lookupFunction<_Run, _Run>(_runName);

typedef _Free1 = Void Function(Pointer<_RunOut>);
typedef _Free2 = void Function(Pointer<_RunOut>);
final _free = getBridgeHandle().lookupFunction<_Free1, _Free2>(_freeName);

String getSocName() {
  final runOut = _run();

  if (runOut.address == 0) {
    throw '$_runName result: nullprt';
  }

  try {
    if (runOut.ref.soc_name.address == 0) {
      throw '$_runName result: data: nullptr';
    }
    return runOut.ref.soc_name.toDartString();
  } finally {
    _free(runOut);
  }
}
