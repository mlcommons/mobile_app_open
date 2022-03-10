class UnsupportedDeviceException implements Exception {
  String backendError;
  UnsupportedDeviceException(this.backendError);
}
