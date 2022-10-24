import 'package:flutter/material.dart';

import 'package:mlperfbench_common/data/build_info/build_info.dart';
import 'package:mlperfbench_common/data/environment/environment_info.dart';
import 'package:mlperfbench_common/data/results/benchmark_result.dart';

import 'package:website/app_state.dart';

class ResultDetailsPage extends StatefulWidget {
  final String id;

  const ResultDetailsPage({Key? key, required this.id}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return ResultDetailsPageState();
  }
}

class ResultDetailsPageState extends State<ResultDetailsPage> {
  Future<void> fetchData() async {
    try {
      await AppState.instance.fetchByUuid(widget.id);
    } catch (e) {
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
    final appState = AppState.instance;
    final result = appState.getByUuid(widget.id);
    if (result == null) {
      return const Center(
        child: CircularProgressIndicator(),
      );
    }
    return Scaffold(
      appBar: AppBar(
        title: const Text('MLPerfBench result browser'),
      ),
      body: SingleChildScrollView(
        child: Padding(
          padding: const EdgeInsets.all(25),
          child: Center(
            child: Container(
              constraints: const BoxConstraints(minWidth: 100, maxWidth: 800),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  _makeTable(<TableRow>[
                    _makeTableRow(
                        const Text('Result UUID'), Text(result.meta.uuid)),
                    // _makeTableRow(const Text('Average throughput'),
                    //     Text('${result.results.calculateAverageThroughput()}')),
                    _makeTableRow(
                        const Text('Upload date'),
                        Text(result.meta.uploadDate!
                            .toLocal()
                            .toIso8601String())),
                  ]),
                  const SizedBox(height: 20),
                  _makeEnvTable(result.environmentInfo),
                  const SizedBox(height: 20),
                  _makeBuildInfoTable(result.buildInfo),
                  const SizedBox(height: 20),
                  ..._makeResults(result.results),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }

  List<Widget> _makeResults(List<BenchmarkExportResult> list) {
    final result = <Widget>[];
    for (var item in list) {
      result.add(_makeBenchTable(item));
      result.add(const SizedBox(height: 20));
    }
    return result;
  }

  Widget _wrapTable(Table table) {
    return Container(
      decoration: BoxDecoration(
        border: Border.all(
          color: Colors.blue,
        ),
        borderRadius: BorderRadius.circular(20),
      ),
      child: Padding(
        padding: const EdgeInsets.all(15.0),
        child: table,
      ),
    );
  }

  Widget _makeEnvTable(EnvironmentInfo info) {
    return _makeTable(
      <TableRow>[
        // _makeTableRow(const Text('Operation system'),
        //     Text(info.osName.toString())), // TODO
        // _makeTableRow(const Text('OS version'), Text(info.osVersion)),
        // _makeTableRow(const Text('Device manufacturer'),
        //     Text(info.manufacturer ?? 'unknown')),
        // _makeTableRow(
        //     const Text('Device model code'), Text(info.modelCode ?? 'unknown')),
        // _makeTableRow(
        //     const Text('Device model name'), Text(info.modelName ?? 'unknown')),
      ],
    );
  }

  Widget _makeBuildInfoTable(BuildInfo info) {
    final originalVersion = info.officialReleaseFlag &&
        !info.devTestFlag &&
        !info.gitDirtyFlag &&
        info.gitBranch == 'master';
    return _makeTable(
      <TableRow>[
        _makeTableRow(const Text('App version'), Text(info.version)),
        _makeTableRow(const Text('Build number'), Text(info.buildNumber)),
        _makeTableRow(const Text('Modified version'),
            Text((!originalVersion).toString())),
      ],
    );
  }

  Widget _makeBenchTable(BenchmarkExportResult info) {
    return _makeTable(
      <TableRow>[
        _makeTableRow(const Text('Benchmark'),
            Text(info.benchmarkName)), // TODO replace with proper name
        _makeTableRow(const Text('Throughput'),
            Text(info.performanceRun?.throughput.toString() ?? 'N/A')),
        _makeTableRow(const Text('Accuracy'),
            Text(info.accuracyRun?.accuracy.toString() ?? 'N/A')),
        _makeTableRow(
            const Text('Backend name'), Text(info.backendInfo.backendName)),
        _makeTableRow(
            const Text('Accelerator'), Text(info.backendInfo.acceleratorName)),
      ],
    );
  }

  Widget _makeTable(List<TableRow> rows) {
    return _wrapTable(Table(
      border: TableBorder.symmetric(
          inside: const BorderSide(width: 1, color: Colors.blue)),
      defaultVerticalAlignment: TableCellVerticalAlignment.middle,
      columnWidths: const {0: FlexColumnWidth(10), 1: FlexColumnWidth(5)},
      children: rows,
    ));
  }

  TableRow _makeTableRow(Widget name, Widget value) {
    return TableRow(
      children: <Widget>[
        Padding(
          padding: const EdgeInsets.all(8.0),
          child: name,
        ),
        Padding(
          padding: const EdgeInsets.all(8.0),
          child: value,
        ),
      ],
    );
  }
}
