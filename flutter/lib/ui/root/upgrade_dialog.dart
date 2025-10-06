import 'dart:io' show Platform;

import 'package:flutter/material.dart';

import 'package:package_info_plus/package_info_plus.dart';
import 'package:upgrader/upgrader.dart';
import 'package:url_launcher/url_launcher.dart';

/// A wrapper widget around [UpgradeAlert] that customizes the "Update now"
/// button behavior. It redirects users to the appropriate store based on the
/// installer source. If the installer is unknown (e.g., sideloaded APK), it
/// opens the project's GitHub Releases page.
class UpgradeDialog extends StatelessWidget {
  final Widget child;

  const UpgradeDialog({super.key, required this.child});

  @override
  Widget build(BuildContext context) {
    final upgrader = Upgrader(
      durationUntilAlertAgain: const Duration(hours: 48),
    );
    return UpgradeAlert(
      upgrader: upgrader,
      onUpdate: () {
        _onUpgradeUpdate();
        return true;
      },
      child: child,
    );
  }

  // Handler for the Upgrader "Update now" button.
  Future<bool> _onUpgradeUpdate() async {
    final uri = await _resolveUpdateUri();
    if (uri == null) return false;
    final can = await canLaunchUrl(uri);
    if (can) {
      await launchUrl(uri, mode: LaunchMode.externalApplication);
      // Return true to indicate we handled navigation and suppress default.
      return true;
    }
    return false;
  }

  // Decide which URL to open for the update action based on installer source.
  Future<Uri?> _resolveUpdateUri() async {
    try {
      final info = await PackageInfo.fromPlatform();
      final installer = info.installerStore;
      final packageName = info.packageName;

      if (Platform.isAndroid) {
        // Map known Android installer package names to their corresponding store URLs.
        final uri = _androidStoreUriFor(installer, packageName);
        if (uri != null) return uri;
      }

      // Fallback/update source for sideloaded APKs or unknown installers.
      // Use the public GitHub releases page for this project.
      return Uri.parse('https://github.com/mlcommons/mobile_app_open/releases');
    } catch (_) {
      // In case of any exception, fall back to the releases page.
      return Uri.parse('https://github.com/mlcommons/mobile_app_open/releases');
    }
  }

  Uri? _androidStoreUriFor(String? installer, String packageName) {
    switch (installer) {
      // Google Play Store
      case 'com.android.vending':
        return Uri.parse(
            'https://play.google.com/store/apps/details?id=$packageName');
      // Amazon Appstore
      case 'com.amazon.venezia':
        return Uri.parse(
            'https://www.amazon.com/gp/mas/dl/android?p=$packageName');
      // Samsung Galaxy Store
      case 'com.sec.android.app.samsungapps':
        return Uri.parse(
            'https://apps.samsung.com/appquery/appDetail.as?appId=$packageName');
      // Huawei AppGallery (best-effort web link; may require app-specific ID)
      case 'com.huawei.appmarket':
        return Uri.parse('https://appgallery.huawei.com/#/app/$packageName');
      // Xiaomi GetApps
      case 'com.xiaomi.mipicks':
        return Uri.parse('https://global.app.mi.com/details?id=$packageName');
      // Oppo App Market
      case 'com.oppo.market':
        return Uri.parse(
            'https://store.heytap.com/app/details?id=$packageName');
      // Vivo App Store
      case 'com.bbk.appstore':
        return Uri.parse('https://appstore.vivo.com.cn/appdetail/$packageName');
      default:
        return null;
    }
  }
}
