import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart' show rootBundle;

import 'package:flutter_markdown/flutter_markdown.dart';
import 'package:url_launcher/url_launcher.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';

class AboutScreen extends StatelessWidget {
  const AboutScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(title: Text(l10n.menuAbout)),
      body: const Center(child: AboutText()),
    );
  }
}

class AboutText extends StatefulWidget {
  const AboutText({super.key});

  @override
  State<AboutText> createState() => _AboutText();
}

class _AboutText extends State<AboutText> {
  final Future<String> _markdownData =
      rootBundle.loadString('assets/text/about.md');

  @override
  Widget build(BuildContext context) {
    return FutureBuilder<String>(
      future: _markdownData,
      builder: (BuildContext context, AsyncSnapshot<String> snapshot) {
        Widget child;
        if (snapshot.hasData) {
          child = Markdown(
            data: snapshot.data!,
            onTapLink: (text, href, title) {
              if (href == null) return;
              final uri = Uri.parse(href);
              unawaited(launchUrl(uri, mode: LaunchMode.externalApplication));
            },
          );
        } else if (snapshot.hasError) {
          child = Text('${snapshot.error}');
        } else {
          child = const CircularProgressIndicator();
        }
        return Center(child: child);
      },
    );
  }
}
