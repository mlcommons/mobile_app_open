# Copyright 2021 The MLPerf Authors. All Rights Reserved.
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

ifeq (${USE_PROXY_WORKAROUND},1)
  export PROXY_WORKAROUND1=\
	-v /etc/ssl/certs:/etc/ssl/certs \
	-v /usr/share/ca-certificates:/usr/share/ca-certificates \
	-v /usr/share/ca-certificates-java:/usr/share/ca-certificates-java

  export PROXY_WORKAROUND2=--host_jvm_args -Djavax.net.ssl.trustStore=/etc/ssl/certs/java/cacerts
endif

ifeq (${OS},Windows_NT)
# On Windows some commands don't run correctly in the default make shell
_start_args=powershell -Command
else
_start_args=
endif
