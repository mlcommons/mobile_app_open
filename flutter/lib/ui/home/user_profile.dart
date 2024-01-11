import 'package:flutter/material.dart';

import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_ui_auth/firebase_ui_auth.dart';
import 'package:provider/provider.dart';

import 'package:mlperfbench/benchmark/state.dart';
import 'package:mlperfbench/firebase/firebase_manager.dart';
import 'package:mlperfbench/localizations/app_localizations.dart';

class UserProfile extends StatefulWidget {
  const UserProfile({Key? key}) : super(key: key);

  @override
  State<StatefulWidget> createState() {
    return _UserProfileState();
  }
}

class _UserProfileState extends State<UserProfile> {
  late BenchmarkState state;
  late AppLocalizations l10n;

  @override
  Widget build(BuildContext context) {
    state = context.watch<BenchmarkState>();
    l10n = AppLocalizations.of(context);

    final currentUser = FirebaseAuth.instance.currentUser;
    final signInWithEmailButton = _buildSignInWithEmailButton();
    final signInAnonymouslyButton = _buildSignInAnonymouslyButton();
    final profileButton = _buildProfileButton();

    List<Widget> children = [];
    if (currentUser == null) {
      children.add(signInAnonymouslyButton);
      children.add(signInWithEmailButton);
    } else {
      final email = currentUser.email;
      if (email != null) {
        children.add(Text(email));
      }
      if (currentUser.isAnonymous) {
        children.add(Text(l10n.userAnonymousUser));
      }
      children.add(const SizedBox(height: 8));
      children.add(profileButton);
    }
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: children,
    );
  }

  Widget _buildSignInAnonymouslyButton() {
    return ElevatedButton(
      onPressed: () {
        FirebaseManager.instance.signInAnonymously();
        Navigator.pop(context);
      },
      child: Text(l10n.userSignInAnonymously),
    );
  }

  Widget _buildSignInWithEmailButton() {
    final resultManager = state.resourceManager.resultManager;
    final signInScreenActions = [
      AuthStateChangeAction<SignedIn>((context, state) {
        resultManager.downloadRemoteResults();
        Navigator.pop(context);
      }),
      AuthStateChangeAction<UserCreated>((context, state) {
        final authCred = state.credential.credential;
        if (authCred != null) {
          FirebaseManager.instance.link(authCred);
        }
        Navigator.pop(context);
      }),
      AuthStateChangeAction<CredentialLinked>((context, state) {
        Navigator.pop(context);
      }),
    ];

    final signInButton = ElevatedButton(
      onPressed: () {
        Navigator.pop(context);
        Navigator.push(
          context,
          MaterialPageRoute(builder: (context) {
            return Scaffold(
              appBar: AppBar(title: Text(l10n.menuSignIn)),
              body: SignInScreen(
                providers: FirebaseManager.instance.authProviders,
                actions: signInScreenActions,
              ),
            );
          }),
        );
      },
      child: Text(l10n.userSignInEmailPassword),
    );
    return signInButton;
  }

  Widget _buildProfileButton() {
    final resultManager = state.resourceManager.resultManager;
    var profileScreenActions = [
      SignedOutAction((context) {
        resultManager.clearRemoteResult();
        Navigator.pop(context);
      })
    ];
    final profileButton = ElevatedButton(
      onPressed: () {
        // FirebaseManager.instance.auth.signOut();
        Navigator.pop(context);
        Navigator.push(
          context,
          MaterialPageRoute(
            builder: (context) {
              return Scaffold(
                appBar: AppBar(title: Text(l10n.menuProfile)),
                body: ProfileScreen(
                  providers: FirebaseManager.instance.authProviders,
                  actions: profileScreenActions,
                  children: [
                    const Divider(),
                    const SizedBox(height: 8),
                    _buildUserInfoSection(),
                    const SizedBox(height: 8),
                    const Divider(),
                  ],
                ),
              );
            },
          ),
        );
      },
      child: Text(l10n.userProfile),
    );
    return profileButton;
  }

  Widget _buildUserInfoSection() {
    final currentUser = FirebaseAuth.instance.currentUser;
    if (currentUser == null) {
      return Text(l10n.unknown);
    }
    List<Widget> children = [];
    final titleTextStyle = Theme.of(context).textTheme.titleMedium;
    final subtitleTextStyle = Theme.of(context).textTheme.bodyMedium;
    const spacing = SizedBox(height: 12);
    final email = currentUser.email;
    if (email != null) {
      children.add(Text(email, style: titleTextStyle));
    }
    if (currentUser.isAnonymous) {
      children.add(Text(l10n.userAnonymousUser, style: titleTextStyle));
    }
    children.add(spacing);

    final userId = Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Text(l10n.userId, style: titleTextStyle),
        Text(currentUser.uid, style: subtitleTextStyle),
      ],
    );
    children.add(userId);

    return Column(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: children,
    );
  }
}
