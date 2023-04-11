#!/bin/sh

set -e

# update git hash
COMMIT_SEARCH_TEMPLATE="[a-f0-9]\{40\}"
COMMIT_HASH=$(git rev-parse @)
sed "s/${COMMIT_SEARCH_TEMPLATE}/${COMMIT_HASH}/g" ios/Runner/Base.lproj/LaunchScreen.template.storyboard > ios/Runner/Base.lproj/LaunchScreen.storyboard

# update version
VERSION=${FLUTTER_APP_VERSION}
VERSION_SEARCH_TEMPLATE="text=\"Version\""
sed -i '' "s/${VERSION_SEARCH_TEMPLATE}/text=\"Version ${VERSION}\"/g" ios/Runner/Base.lproj/LaunchScreen.storyboard
echo "Update splashscreen of iOS app to version: ${VERSION}"
