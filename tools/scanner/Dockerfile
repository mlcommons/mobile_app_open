ARG BASE_DOCKER_IMAGE_TAG
FROM $BASE_DOCKER_IMAGE_TAG

# Install minimum recommended Java version
RUN apt-get update >/dev/null && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    openjdk-17-jdk-headless && \
    rm -rf /var/lib/apt/lists/*

ARG SONAR_SCANNER_DIR=$HOME/.sonar

# Install sonar-scanner
# Check for newer version here: https://binaries.sonarsource.com/?prefix=Distribution/sonar-scanner-cli/
ENV SONAR_SCANNER_VERSION=7.0.2.4839
RUN curl --proto '=https' --create-dirs -sSLo $SONAR_SCANNER_DIR/sonar-scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-$SONAR_SCANNER_VERSION-linux-x64.zip
RUN unzip -qq -o $SONAR_SCANNER_DIR/sonar-scanner.zip -d $SONAR_SCANNER_DIR/
ENV PATH=$PATH:$SONAR_SCANNER_DIR/sonar-scanner-${SONAR_SCANNER_VERSION}-linux-x64/bin
ENV SONAR_SCANNER_OPTS="-server"
RUN sonar-scanner --version

# Install build-wrapper-linux-x86
RUN curl --proto '=https' --create-dirs -sSLo $SONAR_SCANNER_DIR/build-wrapper-linux-x86.zip https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
RUN unzip -qq -o $SONAR_SCANNER_DIR/build-wrapper-linux-x86.zip -d $SONAR_SCANNER_DIR/
ENV PATH=$PATH:$SONAR_SCANNER_DIR/build-wrapper-linux-x86

RUN chmod -R 777 $SONAR_SCANNER_DIR

# Install Node.js and NPM for JS analysis
ENV NODE_ENV="production"
RUN set -uex; \
    apt-get update >/dev/null; \
    apt-get install -y --no-install-recommends ca-certificates curl gnupg; \
    apt-get clean; \
    mkdir -p /etc/apt/keyrings; \
    curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key \
     | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg; \
    NODE_MAJOR=18; \
    echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" \
     > /etc/apt/sources.list.d/nodesource.list; \
    apt-get update >/dev/null; \
    apt-get install -y --no-install-recommends nodejs; \
    rm -rf /var/lib/apt/lists/*
