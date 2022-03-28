import 'package:flutter/material.dart';

import 'package:website/app_state.dart';
import 'package:website/route_generator.dart';

class ResultsListPage extends StatefulWidget {
  final String fromUuid;
  const ResultsListPage({Key? key, required this.fromUuid}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return ResultsListPageState();
  }
}

class ResultsListPageState extends State<ResultsListPage> {
  Future<void> fetchData() async {
    try {
      await AppState.instance.fetchBatch(from: widget.fromUuid);
    } catch (e, s) {
      print(e);
      return;
    }
    if (mounted) {
      setState(() {});
    }
  }

  @override
  void initState() {
    super.initState();
    fetchData();
  }

  @override
  Widget build(BuildContext context) {
    final state = AppState.instance;
    final ids = state.getBatch(from: widget.fromUuid);
    if (ids == null) {
      return const Center(
        child: CircularProgressIndicator(),
      );
    }
    return Scaffold(
      appBar: AppBar(
        title: const Text('MLPerfBench result browser'),
      ),
      body: SingleChildScrollView(
        child: Center(
          child: Container(
            constraints: BoxConstraints(minWidth: 100, maxWidth: 800),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                const Padding(
                  padding: EdgeInsets.all(20),
                  child: Text(
                    'Latest results',
                    style: TextStyle(
                      fontSize: 50.0,
                    ),
                  ),
                ),
                _createList(state, ids),
                Row(
                  children: [
                    _createButton('First / Refresh List', false, () {
                      Navigator.of(context)
                          .pushNamed('/${AppRoutes.resultsList}/');
                    }),
                    _createButton('Prev', true, () {}), // TODO
                    _createButton('Next', ids.length < AppState.pageSize, () {
                      final id = ids[ids.length - 1];
                      Navigator.of(context)
                          .pushNamed('/${AppRoutes.resultsList}/$id');
                    }),
                  ],
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _createButton(String text, bool disabled, void Function() onPressed) {
    return Padding(
        padding: EdgeInsets.fromLTRB(10, 10, 10, 10),
        child: TextButton(
          style: ButtonStyle(
              backgroundColor: MaterialStateProperty.all<Color>(
                  disabled ? Colors.white : Colors.green),
              shape: MaterialStateProperty.all(RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(14.0),
                  side: BorderSide(color: Colors.green))),
              minimumSize: MaterialStateProperty.all<Size>(Size(100, 0))),
          onPressed: disabled ? null : onPressed,
          child: Padding(
            padding: EdgeInsets.fromLTRB(0, 10, 0, 10),
            child: Text(
              text,
              style: TextStyle(
                  fontSize: 20.0, color: disabled ? Colors.grey : Colors.white),
            ),
          ),
        ));
  }

  Widget _createList(AppState state, List<String> ids) {
    return ListView.builder(
        shrinkWrap: true,
        itemCount: ids.length, // TODO properly handle case size == 0
        itemBuilder: (context, index) {
          final details = state.getByUuid(ids[index])!;
          final averageScoreText =
              'Average score: ${details.results.calculateAverageThroughput()}}';
          final uploadDateText = 'Upload date: ${details.uploadDate}';
          return Card(
              child: ListTile(
            title: Text(details.uuid),
            isThreeLine: true,
            subtitle: Text('$averageScoreText\n$uploadDateText'),
            onTap: () {
              Navigator.of(context)
                  .pushNamed('/${AppRoutes.resultDetails}/${details.uuid}');
            },
          ));
        });
  }
}
