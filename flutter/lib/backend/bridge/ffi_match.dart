import 'dart:ffi';
import 'dart:io' show Platform;

import 'package:device_info/device_info.dart';
import 'package:ffi/ffi.dart';

import 'package:mlcommons_ios_app/protos/backend_setting.pb.dart' as pb;
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
    Pointer<Utf8> lib_path, Pointer<Utf8> manufacturer, Pointer<Utf8> model);
final _run = getBridgeHandle().lookupFunction<_Run, _Run>(_runName);

typedef _Free1 = Void Function(Pointer<_RunOut>);
typedef _Free2 = void Function(Pointer<_RunOut>);
final _free = getBridgeHandle().lookupFunction<_Free1, _Free2>(_freeName);

late final Pointer<Utf8> _manufacturerUtf8;
late final Pointer<Utf8> _modelUtf8;

Future<void> initDeviceInfo() async {
  String model;
  String manufacturer;
  if (Platform.isIOS) {
    final deviceInfo = DeviceInfoPlugin();
    final iosInfo = await deviceInfo.iosInfo;

    manufacturer = 'Apple';
    model = iosInfo.name;
  } else if (Platform.isWindows) {
    manufacturer = 'Microsoft';
    model = 'Unknown PC';
  } else if (Platform.isAndroid) {
    final deviceInfo = await DeviceInfoPlugin().androidInfo;

    manufacturer = deviceInfo.manufacturer;
    model = deviceInfo.model;
  } else {
    throw 'Could not define platform';
  }
  _manufacturerUtf8 = manufacturer.toNativeUtf8();
  _modelUtf8 = model.toNativeUtf8();
}

pb.BackendSetting? backendMatch(String libPath) {
  final libPathUtf8 = libPath.toNativeUtf8();
  final runOut = _run(libPathUtf8, _manufacturerUtf8, _modelUtf8);
  malloc.free(libPathUtf8);

  if (runOut.address == 0) {
    return null;
  }
  if (runOut.ref.matches == 0) {
    return null;
  }
  if (runOut.ref.pbdata.address == 0) {
    throw '$_runName result: pbdata: nullptr';
  }

  try {
    final view = runOut.ref.pbdata.asTypedList(runOut.ref.pbdata_size);
    return pb.BackendSetting.fromBuffer(view);
  } finally {
    _free(runOut);
  }
}
