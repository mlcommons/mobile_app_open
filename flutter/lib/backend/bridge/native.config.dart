part of 'native.dart';

class _MLPerfConfigResult extends Struct {
  @Int32()
  external int size;

  external Pointer<Uint8> data;
}

typedef _MLPerfConfig = Pointer<_MLPerfConfigResult> Function(
    Pointer<Utf8> pb_content);

typedef _MLPerfConfigFree1 = Void Function(Pointer<_MLPerfConfigResult>);
typedef _MLPerfConfigFree2 = void Function(Pointer<_MLPerfConfigResult>);

final _mlperf_config = _bridge
    .lookupFunction<_MLPerfConfig, _MLPerfConfig>('dart_ffi_mlperf_config');
final _mlperf_config_free =
    _bridge.lookupFunction<_MLPerfConfigFree1, _MLPerfConfigFree2>(
        'dart_ffi_mlperf_config_free');

pb.MLPerfConfig getMLPerfConfig(String pbtxtContent) {
  final pbtxtContent_ = pbtxtContent.toNativeUtf8();
  final pbdata = _mlperf_config(pbtxtContent_);

  pb.MLPerfConfig? config;

  try {
    if (pbdata.ref.data.address != 0) {
      final convertedContent = pbdata.ref.data.asTypedList(pbdata.ref.size);
      config = pb.MLPerfConfig.fromBuffer(convertedContent);
    }
  } finally {
    _mlperf_config_free(pbdata);
    malloc.free(pbtxtContent_);
  }
  if (config == null) {
    throw 'Could not load content from pbtxt file';
  }

  return config;
}
