import 'package:flutter/material.dart';

import 'ui/not_found.dart';
import 'ui/result_details.dart';
import 'ui/home.dart';
import 'ui/results_list.dart';

class AppRoutes {
  static const String home = 'home';
  static const String resultsList = 'results';
  static const String resultDetails = 'result';
}

class RouteGenerator {
  static Route<dynamic> generateRoute(RouteSettings settings) {
    final uri = Uri.parse(settings.name!);
    final page =
        uri.pathSegments.isEmpty ? AppRoutes.home : uri.pathSegments[0];

    print('switching on $page');
    switch (page) {
      case AppRoutes.home:
        return MaterialPageRoute(
            settings: settings, builder: (_) => const HomePage());
      case AppRoutes.resultsList:
        final from = uri.pathSegments.length >= 2 ? uri.pathSegments[1] : '';
        return MaterialPageRoute(
            settings: settings,
            builder: (_) => ResultsListPage(fromUuid: from));
      case AppRoutes.resultDetails:
        if (uri.pathSegments.length < 2 || uri.pathSegments[1] == '') {
          return MaterialPageRoute(
              settings: settings, builder: (_) => const NotFoundPage());
        }
        return MaterialPageRoute(
            settings: settings,
            builder: (_) => ResultDetailsPage(id: uri.pathSegments[1]));
      default:
        return MaterialPageRoute(
            settings: settings, builder: (_) => const NotFoundPage());
    }
  }
}
