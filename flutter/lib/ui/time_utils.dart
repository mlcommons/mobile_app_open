String formatDuration(int milliseconds) {
  var seconds = (milliseconds / Duration.millisecondsPerSecond).ceil();
  var minutes = seconds ~/ Duration.secondsPerMinute;
  seconds -= minutes * Duration.secondsPerMinute;
  final hours = minutes ~/ Duration.minutesPerHour;
  minutes -= hours * Duration.minutesPerHour;

  final tokens = <String>[];
  if (hours != 0) {
    tokens.add(hours.toString().padLeft(2, '0'));
  }
  tokens.add(minutes.toString().padLeft(2, '0'));
  tokens.add(seconds.toString().padLeft(2, '0'));

  return tokens.join(':');
}
