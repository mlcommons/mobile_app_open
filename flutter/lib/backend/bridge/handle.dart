import 'dart:ffi';
import 'dart:io';

DynamicLibrary _getBridgeLibraryHandle() {
  if (Platform.isWindows) {
    return DynamicLibrary.open('./libs/backend_bridge.dll');
  } else if (Platform.isIOS) {
    // return DynamicLibrary.process();
    return DynamicLibrary.open('backend_bridge_fw.framework/backend_bridge_fw');
  } else if (Platform.isAndroid) {
    return DynamicLibrary.open('libbackendbridge.so');
  } else {
    throw 'unsupported platform';
  }
}

final DynamicLibrary _bridge = _getBridgeLibraryHandle();

DynamicLibrary getBridgeHandle() {
  return _bridge;
}
