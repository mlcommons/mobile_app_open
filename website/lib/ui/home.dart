import 'package:flutter/material.dart';

import 'package:website/route_generator.dart';

class HomePage extends StatelessWidget {
  const HomePage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('MLPerfBench result browser'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'Hi, this is the home page of MLPerfBench website',
              style: TextStyle(fontSize: 30),
            ),
            const SizedBox(height: 20),
            ElevatedButton(
                child: const Text(
                  'Take me to results',
                ),
                onPressed: () {
                  Navigator.of(context).pushNamed('/${AppRoutes.resultsList}/');
                })
          ],
        ),
      ),
    );
  }
}
