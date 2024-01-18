import 'package:flutter/material.dart';

import 'package:mlperfbench/data/result_file_name.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/ui/history/extended_result_screen.dart';
import 'package:mlperfbench/ui/time_utils.dart';

class UploadedFilesScreen extends StatefulWidget {
  const UploadedFilesScreen({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() => _UploadedFilesScreenState();
}

class _UploadedFilesScreenState extends State<UploadedFilesScreen> {
  Future<List<String>> fetchFileList() =>
      FirebaseManager.instance.listResults();

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context);
    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.userUploadedFiles),
      ),
      body: FutureBuilder(
        future: fetchFileList(),
        builder: (context, AsyncSnapshot<List<String>> snapshot) {
          Widget child;
          final fileList = snapshot.data;
          if (snapshot.hasData && fileList != null) {
            fileList.sort((b, a) => a.compareTo(b));
            child = ListView.separated(
              itemCount: fileList.length,
              shrinkWrap: false,
              separatorBuilder: (_, __) => const Divider(),
              itemBuilder: (context, index) {
                final fileName = fileList[index];
                final resultFileName = ResultFileName.fromFileName(fileName);
                return ListTile(
                  title: Padding(
                    padding: const EdgeInsets.fromLTRB(0, 8, 0, 4),
                    child: Text(resultFileName.dateTime.toUIString()),
                  ),
                  subtitle: Text(fileName),
                  onTap: () {
                    _viewFile(fileName);
                  },
                  trailing: IconButton(
                    icon: const Icon(Icons.delete),
                    onPressed: () async {
                      await _deleteFile(fileName);
                    },
                  ),
                );
              },
            );
          } else if (snapshot.hasError) {
            child = Text('${snapshot.error}');
          } else {
            child = const CircularProgressIndicator();
          }
          return Center(child: child);
        },
      ),
    );
  }

  Future<void> _deleteFile(String fileName) async {
    await FirebaseManager.instance.deleteResult(fileName);
    setState(() {
      fetchFileList();
    });
  }

  void _viewFile(String fileName) {
    Navigator.push(context, MaterialPageRoute(
      builder: (context) {
        return RemoteExtendedResultScreen(fileName: fileName);
      },
    )).then((value) => setState(() {
          fetchFileList();
        }));
  }
}
