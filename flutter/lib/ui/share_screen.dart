import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';

import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';

import 'package:mlperfbench/app_constants.dart';
import 'package:mlperfbench/icons.dart' show AppIcons;
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'package:mlperfbench/ui/page_constraints.dart';
import 'main_screen.dart';

class ShareKeys {
  // list of widget keys that need to be accessed in the test code
  static const String shareButton = 'shareButton';
  static const String doNotShareButton = 'doNotShareButton';
}

class ShareScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    final store = context.watch<Store>();
    final stringResources = AppLocalizations.of(context);

    final iconEdgeSize = MediaQuery.of(context).size.width * 0.66;
    final minimumShareButtonWidth = MediaQuery.of(context).size.width - 60;

    final shareButtonStyle = ButtonStyle(
        backgroundColor:
            MaterialStateProperty.all<Color>(AppColors.shareRectangle),
        shape: MaterialStateProperty.all(RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(14.0),
            side: BorderSide(color: Colors.white))),
        minimumSize:
            MaterialStateProperty.all<Size>(Size(minimumShareButtonWidth, 0)));

    return Scaffold(
      body: getSinglePageView(
        Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            Expanded(
                flex: 3,
                child: Column(
                    crossAxisAlignment: CrossAxisAlignment.center,
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Expanded(
                          flex: 3,
                          child: Align(
                              alignment: Alignment.bottomCenter,
                              child: Container(
                                  height: iconEdgeSize,
                                  width: iconEdgeSize,
                                  child: AppIcons.logo))),
                      Expanded(
                          child: Padding(
                              padding: EdgeInsets.fromLTRB(35, 0, 35, 0),
                              child: Align(
                                  alignment: Alignment.topCenter,
                                  child: Text(stringResources.privacyPrompt,
                                      textAlign: TextAlign.center,
                                      style: TextStyle(
                                          fontSize: 15,
                                          color: AppColors.darkText)))))
                    ])),
            Expanded(
              flex: 2,
              child: Column(
                children: [
                  Expanded(
                      flex: 2,
                      child: Column(children: [
                        Padding(
                            padding: EdgeInsets.fromLTRB(10, 10, 10, 10),
                            child: TextButton(
                                key: Key(ShareKeys.shareButton),
                                style: shareButtonStyle,
                                onPressed: () {
                                  store.share = true;

                                  Navigator.pushReplacement(
                                    context,
                                    MaterialPageRoute(
                                        builder: (context) => MyHomePage()),
                                  );
                                },
                                child: Padding(
                                  padding: EdgeInsets.fromLTRB(0, 10, 0, 10),
                                  child: Text(
                                    stringResources.share,
                                    style: TextStyle(
                                        fontSize: 20,
                                        color: AppColors.lightText),
                                  ),
                                ))),
                        Padding(
                            padding: EdgeInsets.only(top: 15),
                            child: TextButton(
                              key: Key(ShareKeys.doNotShareButton),
                              onPressed: () {
                                store.share = false;

                                Navigator.pushReplacement(
                                  context,
                                  MaterialPageRoute(
                                      builder: (context) => MyHomePage()),
                                );
                              },
                              child: Text(
                                stringResources.dontShare,
                                style: TextStyle(
                                  color: AppColors.shareTextButton,
                                  fontSize: 18,
                                ),
                              ),
                            ))
                      ])),
                  Expanded(
                    child: Padding(
                      padding: EdgeInsets.fromLTRB(35, 20, 35, 20),
                      child: RichText(
                        text: TextSpan(
                          text: stringResources.promptSubText,
                          style: TextStyle(fontSize: 13, color: Colors.grey),
                          children: [
                            TextSpan(
                              text: stringResources.privacyPolicyButton,
                              style: TextStyle(
                                  fontSize: 13,
                                  color: AppColors.shareTextButton),
                              recognizer: TapGestureRecognizer()
                                ..onTap = () => _launchURL(),
                            ),
                          ],
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}

Future<void> _launchURL() async {
  const url = 'https://mlcommons.org/mobile_privacy';
  if (await canLaunch(url)) {
    await launch(url);
  } else {
    throw 'Could not launch $url';
  }
}
