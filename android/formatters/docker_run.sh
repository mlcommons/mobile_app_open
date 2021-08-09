#!/bin/bash
docker run -it \
                -e USER=mlperf \
                -v `pwd`:/home/mlperf/mobile_app \
                -w /home/mlperf/mobile_app \
                -u `id -u`:`id -g` \
                mlcommons/mlperf_mobile:1.0 $*

