import 'dart:io';

import 'package:flutter/material.dart' hide Icons;
import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:mlperfbench/localizations/app_localizations.dart';
import 'package:mlperfbench/store.dart';
import 'config_batch_screen.dart';

class ConfigScreen extends StatefulWidget {
  @override
  _ConfigScreen createState() => _ConfigScreen();
}

class _ConfigScreen extends State<ConfigScreen> {
  @override
  Widget build(BuildContext context) {
    final store = context.watch<Store>();
    final stringResources = AppLocalizations.of(context);
    final childrenList = <Widget>[];

    for (var item in store.getBenchmarkList()) {
      childrenList.add(ListTile(
        title: Padding(
          padding: const EdgeInsets.only(bottom: 5),
          child: Text(
            item.name,
          ),
        ),
        subtitle: Text(item.id + ' | ' + item.description),
        leading: Checkbox(
            value: item.active,
            onChanged: (bool? value) {
              setState(() {
                item.active = value == true ? true : false;
              });
            }),
        trailing: item.batchSize > 0 && !Platform.isAndroid
            ? IconButton(
                icon: Icon(Icons.settings),
                tooltip: 'Batch settings',
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute(
                        builder: (context) => ConfigBatchScreen(item.id)),
                  );
                })
            : null,
        onTap: () {
          setState(() {
            item.active = !item.active;
          });
        },
      ));
    }

    return Scaffold(
      appBar: AppBar(
        title: Text(
          stringResources.configTitle,
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.only(top: 20),
        children: childrenList,
      ),
    );
  }
}
