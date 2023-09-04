# Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

ifeq (${WITH_INTERNAL_DOCKER_OPTION},1)
host_ip=$(shell nslookup docker-registry.qualcomm.com | grep -n Address | grep ^8 | cut -c12-)
internal_docker_option=--add-host=docker:${host_ip}
else
internal_docker_option=
endif