import 'package:flutter/material.dart';

class NotFoundPage extends StatelessWidget {
  const NotFoundPage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Page not found'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              "The page you requested doesn't exist",
              style: TextStyle(fontSize: 30),
            ),
            const SizedBox(height: 20),
            ElevatedButton(
                child: const Text(
                  'Go to home page',
                ),
                onPressed: () {
                  Navigator.of(context).pushNamed('/');
                })
          ],
        ),
      ),
    );
  }
}
