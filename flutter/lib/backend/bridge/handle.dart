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

String libPathFromName(String libName) {
  if (Platform.isAndroid) {
    return '$libName.so';
  } else if (Platform.isIOS) {
    return '$libName.framework/$libName';
  } else if (Platform.isWindows) {
    return '$libName.dll';
  } else {
    throw 'unsupported platform';
  }
}
