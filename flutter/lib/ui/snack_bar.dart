import 'package:flutter/material.dart';

SnackBar getSnackBar(String message) => SnackBar(
      duration: Duration(seconds: 2),
      backgroundColor: Color(0xFFEDEDED),
      content: Text(
        message,
        textAlign: TextAlign.center,
        style: TextStyle(color: Colors.black),
      ),
    );
