FROM mlcommons/mlperf_mobile_flutter

RUN apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y unzip

ARG SONAR_SCANNER_DIR=$HOME/.sonar

# Install sonar-scanner
ENV SONAR_SCANNER_VERSION=4.7.0.2747
RUN curl --create-dirs -sSLo $SONAR_SCANNER_DIR/sonar-scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-$SONAR_SCANNER_VERSION-linux.zip
RUN unzip -qq -o $SONAR_SCANNER_DIR/sonar-scanner.zip -d $SONAR_SCANNER_DIR/
ENV PATH=$PATH:$SONAR_SCANNER_DIR/sonar-scanner-${SONAR_SCANNER_VERSION}-linux/bin
ENV SONAR_SCANNER_OPTS="-server"
RUN sonar-scanner --version

# Add Flutter plugin
RUN curl --create-dirs -sSLo $SONAR_SCANNER_DIR/sonar-scanner-${SONAR_SCANNER_VERSION}-linux/extensions/plugins/sonar-flutter-plugin-0.4.0.jar https://github.com/insideapp-oss/sonar-flutter/releases/download/0.4.0/sonar-flutter-plugin-0.4.0.jar

# The base image `mlcommons/mlperf_mobile_flutter` does not have dart in path
ENV PATH=$PATH:$HOME/flutter/bin/cache/dart-sdk/bin

# Install build-wrapper-linux-x86
RUN curl --create-dirs -sSLo $SONAR_SCANNER_DIR/build-wrapper-linux-x86.zip https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
RUN unzip -qq -o $SONAR_SCANNER_DIR/build-wrapper-linux-x86.zip -d $SONAR_SCANNER_DIR/
ENV PATH=$PATH:$SONAR_SCANNER_DIR/build-wrapper-linux-x86

RUN chmod -R 777 $SONAR_SCANNER_DIR

# Install Node.js and NPM for JS analysis
ENV NODE_ENV="production"
RUN curl -fsSL https://deb.nodesource.com/setup_lts.x | bash -
RUN apt-get update && apt-get install -y nodejs && node -v
RUN npm install --location=global npm@8.13.2 && npm --version
