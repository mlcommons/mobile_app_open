import 'package:flutter/material.dart';

import 'package:collection/collection.dart';

import 'package:mlperfbench/benchmark/benchmark.dart';
import 'package:mlperfbench/ui/app_styles.dart';

// Labels come from each backend's pbtxt framework value; when two backends
// declare the same framework name, append a cleaned libName to disambiguate.
Map<String, String> backendChoiceLabels(Benchmark benchmark) {
  final frameworks = benchmark.backends
      .map((b) => b.settings.framework)
      .toList();
  final labels = <String, String>{};
  for (final b in benchmark.backends) {
    final framework = b.settings.framework;
    final collision = frameworks.where((f) => f == framework).length > 1;
    if (collision) {
      final cleaned = b.info.libName
          .replaceFirst(RegExp('^lib'), '')
          .replaceFirst(RegExp(r'backend$'), '');
      labels[b.info.libName] = '$framework ($cleaned)';
    } else {
      labels[b.info.libName] = framework;
    }
  }
  return labels;
}

/// Whether the selected backend offers a real delegate choice. Mirrors the
/// cases [DelegateChoice] renders nothing for.
bool hasDelegateChoice(Benchmark benchmark) {
  final choices = benchmark.benchmarkSettings.delegateChoice;
  if (choices.isEmpty) return false;
  if (choices.length == 1 && choices.first.delegateName.isEmpty) return false;
  return true;
}

class BackendChoice extends StatelessWidget {
  final Benchmark benchmark;
  final ValueChanged<String> onChanged;

  const BackendChoice({
    super.key,
    required this.benchmark,
    required this.onChanged,
  });

  @override
  Widget build(BuildContext context) {
    final style = Theme.of(context).textTheme.labelLarge;
    if (benchmark.backends.length <= 1) {
      return Text(
        benchmark.backendRequestDescription,
        style: style,
        maxLines: 1,
        overflow: TextOverflow.ellipsis,
        softWrap: false,
      );
    }
    final labels = backendChoiceLabels(benchmark);
    return SizedBox(
      height: 24,
      child: DropdownButton<String>(
        isExpanded: false,
        isDense: false,
        padding: const EdgeInsets.only(left: 6),
        icon: const Icon(Icons.expand_more_rounded),
        borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
        underline: const SizedBox(),
        value: benchmark.selectedBackend.info.libName,
        items: benchmark.backends
            .map(
              (b) => DropdownMenuItem<String>(
                value: b.info.libName,
                child: Text(labels[b.info.libName]!, style: style),
              ),
            )
            .toList(),
        onChanged: (value) {
          if (value != null) onChanged(value);
        },
        style: style,
      ),
    );
  }
}

class DelegateChoice extends StatelessWidget {
  final Benchmark benchmark;
  final ValueChanged<String> onChanged;

  const DelegateChoice({
    super.key,
    required this.benchmark,
    required this.onChanged,
  });

  @override
  Widget build(BuildContext context) {
    final selected = benchmark.benchmarkSettings.delegateSelected;
    final choices = benchmark.benchmarkSettings.delegateChoice
        .sorted((b, a) => a.priority.compareTo(b.priority))
        .map((e) => e.delegateName)
        .toList();
    if (!hasDelegateChoice(benchmark)) {
      return const SizedBox.shrink();
    }
    if (!choices.contains(selected)) {
      throw 'delegate_selected=$selected must be one of delegate_choice=$choices';
    }
    return SizedBox(
      height: 24,
      child: DropdownButton<String>(
        isExpanded: false,
        isDense: false,
        padding: const EdgeInsets.only(left: 6),
        icon: const Icon(Icons.expand_more_rounded),
        borderRadius: BorderRadius.circular(WidgetSizes.borderRadius),
        underline: const SizedBox(),
        value: selected,
        items: choices
            .map(
              (item) => DropdownMenuItem<String>(
                value: item,
                child: Text(
                  item,
                  style: Theme.of(context).textTheme.labelLarge,
                ),
              ),
            )
            .toList(),
        onChanged: (value) => onChanged(value ?? ''),
        style: Theme.of(context).textTheme.labelLarge,
      ),
    );
  }
}
