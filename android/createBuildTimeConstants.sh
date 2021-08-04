#!/bin/sh
FILE="java/org/mlperf/inference/BuildTimeConstants.java"
echo "package org.mlperf.inference;" >$FILE
echo "public class BuildTimeConstants {">>$FILE
echo "public static final String HASH = \"${GIT_COMMIT}\";">>$FILE
echo "}">>$FILE
