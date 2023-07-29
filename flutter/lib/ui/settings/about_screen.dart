import 'package:flutter/material.dart';
import 'package:flutter/services.dart' show rootBundle;

import 'package:flutter_markdown/flutter_markdown.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';

class AboutScreen extends StatelessWidget {
  const AboutScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    return Scaffold(
      appBar: AppBar(title: Text(l10n.menuAbout)),
      body: const Center(child: AboutText()),
    );
  }
}

class AboutText extends StatefulWidget {
  const AboutText({Key? key}) : super(key: key);

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
          child = Markdown(data: snapshot.data!);
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
