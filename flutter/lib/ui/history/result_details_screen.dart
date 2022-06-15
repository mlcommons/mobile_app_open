import 'package:flutter/material.dart';
import 'package:mlperfbench_common/data/extended_result.dart';

// import 'package:intl/intl.dart';

import 'package:tuple/tuple.dart';
// import 'package:mlperfbench/localizations/app_localizations.dart';

class DetailsScreen extends StatefulWidget {
  final ExtendedResult result;

  const DetailsScreen({Key? key, required this.result}) : super(key: key);

  @override
  _DetailsScreen createState() => _DetailsScreen();
}

class _DetailsScreen extends State<DetailsScreen> {
  @override
  Widget build(BuildContext context) {
    // final stringResources = AppLocalizations.of(context);

    // var dateFormat = DateFormat('yyyy-MM-dd HH:mm');

    return Scaffold(
      appBar: AppBar(
        title: Text(
          'Result details', // TODO move to resources
        ),
      ),
      body: ListView(padding: const EdgeInsets.only(top: 20), children: [
        _makeResultTable(),
      ]),
    );
  }

  Widget _makeResultTable() {
    return _makeTable(
      [
            Tuple2([
              'Benchmark name',
              'Throughput',
              'Accuracy',
              // ignore: unnecessary_cast
            ], null as void Function()?)
          ] +
          widget.result.results.list.map((runInfo) {
            return Tuple2([
              runInfo.benchmarkName,
              runInfo.performance?.throughput?.toStringAsFixed(2) ?? 'N/A',
              runInfo.accuracy?.accuracy?.formatted ?? 'N/A',
            ], () {});
          }).toList(),
    );
  }

  Widget _makeTable(List<Tuple2<List<String>, void Function()?>> rows) {
    return (Column(
      children: rows.map<Widget>((rowData) {
        final rowValues = rowData.item1;
        final onTap = rowData.item2;
        final table = Table(
          border: TableBorder.all(
            width: 1,
            color: Colors.blue,
          ),
          defaultVerticalAlignment: TableCellVerticalAlignment.middle,
          columnWidths: const {
            0: FlexColumnWidth(10),
            1: FlexColumnWidth(5),
            2: FlexColumnWidth(5),
          },
          children: [
            _makeTableRow(rowValues.map((value) {
              return Text(
                value,
                style: onTap == null
                    ? TextStyle(fontWeight: FontWeight.bold)
                    : null,
              );
            }).toList())
          ],
        );
        return InkWell(
          onTap: onTap,
          child: table,
        );
      }).toList(),
    ));
  }

  TableRow _makeTableRow(List<Widget> cells) {
    return TableRow(
      children: cells.map(
        (cell) {
          return Padding(
            padding: EdgeInsets.all(8.0),
            child: cell,
          );
        },
      ).toList(),
    );
  }
}
