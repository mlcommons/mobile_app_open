// ignore_for_file: non_constant_identifier_names

import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'package:mlperfbench/backend/unsupported_device_exception.dart';
import 'package:mlperfbench/device_info.dart';
import 'package:mlperfbench/protos/backend_setting.pb.dart' as pb;
import 'handle.dart';

class _RunOut extends Struct {
  @Int32()
  external int matches;
  external Pointer<Utf8> error_message;

  @Int32()
  external int pbdata_size;
  external Pointer<Uint8> pbdata;
}

const _runName = 'dart_ffi_backend_match';
const _freeName = 'dart_ffi_backend_match_free';

typedef _Run = Pointer<_RunOut> Function(
    Pointer<Utf8> libPath, Pointer<Utf8> manufacturer, Pointer<Utf8> model);
final _run = getBridgeHandle().lookupFunction<_Run, _Run>(_runName);

typedef _Free1 = Void Function(Pointer<_RunOut>);
typedef _Free2 = void Function(Pointer<_RunOut>);
final _free = getBridgeHandle().lookupFunction<_Free1, _Free2>(_freeName);

pb.BackendSetting? backendMatch(String libPath) {
  final libPathUtf8 = libPath.toNativeUtf8();
  final manufacturerUtf8 = DeviceInfo.manufacturer.toNativeUtf8();
  final modelUtf8 = DeviceInfo.model.toNativeUtf8();
  final runOut = _run(libPathUtf8, manufacturerUtf8, modelUtf8);
  malloc.free(libPathUtf8);
  malloc.free(manufacturerUtf8);
  malloc.free(modelUtf8);

  if (runOut.address == 0) {
    return null;
  }

  try {
    if (runOut.ref.error_message.address != 0) {
      final errorMessage = runOut.ref.error_message.toDartString();
      throw UnsupportedDeviceException(errorMessage);
    }
    if (runOut.ref.matches == 0) {
      return null;
    }
    if (runOut.ref.pbdata.address == 0) {
      throw '$_runName result: pbdata: nullptr';
    }

    final view = runOut.ref.pbdata.asTypedList(runOut.ref.pbdata_size);
    return pb.BackendSetting.fromBuffer(view);
  } finally {
    _free(runOut);
  }
}
