# Copyright (c) 2020-2021 Qualcomm Innovation Center, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

this_mkfile:=$(abspath $(lastword $(MAKEFILE_LIST)))
DLCDIR:=$(abspath $(shell dirname ${this_mkfile})/../DLC)
TOPDIR=$(abspath ${DLCDIR}/../..)
BUILDDIR=${TOPDIR}/output
SNPE_VERSION=$(shell grep SNPE_VERSION ../variables.bzl | cut -d\" -f2)

USERID=$(shell id -u)
GROUPID=$(shell id -g)

