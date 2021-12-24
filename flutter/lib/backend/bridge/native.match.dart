part of 'native.dart';

class _BackendMatchResult extends Struct {
  @Int32()
  external int matches;
  external Pointer<Utf8> error_message;

  @Int32()
  external int pbdata_size;
  external Pointer<Uint8> pbdata;
}

typedef _BackendMatch = Pointer<_BackendMatchResult> Function(
    Pointer<Utf8> lib_path, Pointer<Utf8> manufacturer, Pointer<Utf8> model);

typedef _BackendMatchFree1 = Void Function(Pointer<_BackendMatchResult>);
typedef _BackendMatchFree2 = void Function(Pointer<_BackendMatchResult>);

final _backend_match = _bridge
    .lookupFunction<_BackendMatch, _BackendMatch>('dart_ffi_backend_match');
final _backend_match_free =
    _bridge.lookupFunction<_BackendMatchFree1, _BackendMatchFree2>(
        'dart_ffi_backend_match_free');

Future<pb.BackendSetting?> backendMatch(String lib_path) async {
  Pointer<Utf8> model;
  Pointer<Utf8> manufacturer;

  if (Platform.isIOS) {
    final deviceInfo = DeviceInfoPlugin();
    final iosInfo = await deviceInfo.iosInfo;

    manufacturer = 'Apple'.toNativeUtf8();
    model = iosInfo.name.toNativeUtf8();
  } else if (Platform.isWindows) {
    manufacturer = 'Microsoft'.toNativeUtf8();
    model = 'Unknown PC'.toNativeUtf8();
  } else if (Platform.isAndroid) {
    final deviceInfo = await DeviceInfoPlugin().androidInfo;

    manufacturer = deviceInfo.manufacturer.toNativeUtf8();
    model = deviceInfo.model.toNativeUtf8();
  } else {
    throw 'Could not define platform';
  }

  final lib_path_ = lib_path.toNativeUtf8();
  final matchResult = _backend_match(lib_path_, manufacturer, model);
  malloc.free(lib_path_);
  malloc.free(manufacturer);
  malloc.free(model);

  if (matchResult.address == 0) {
    return null;
  }

  try {
    if (matchResult.ref.matches != 0 && matchResult.ref.pbdata.address != 0) {
      final view =
          matchResult.ref.pbdata.asTypedList(matchResult.ref.pbdata_size);
      return pb.BackendSetting.fromBuffer(view);
    }
  } finally {
    _backend_match_free(matchResult);
  }

  return null;
}
