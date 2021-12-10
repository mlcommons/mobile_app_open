package org.mlcommons.android.mlperfbench

import androidx.annotation.NonNull
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.embedding.android.FlutterActivity
import io.flutter.plugin.common.MethodChannel

class MainActivity: FlutterActivity() {
    private val CHANNEL_TAG = "org.mlcommons.mlperfbench/android"

    override fun configureFlutterEngine(@NonNull flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL_TAG).setMethodCallHandler {
          call, result ->
            if (call.method == "getNativeLibraryPath") {
                result.success(getContext().getApplicationInfo().nativeLibraryDir);
            } else {
                result.notImplemented();
            }
        }
      }
}
