# Copyright (c) 2020-2025 Qualcomm Innovation Center, Inc. All rights reserved.
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

import os

url=os.sys.argv[1]
numbytes=int(os.sys.argv[2])
prefix=os.sys.argv[3]

tokenstr = ""
if len(os.sys.argv) > 4 and os.sys.argv[4] == "usetoken":
  tokenstr = '-H "Authorization: token ${GITHUB_TOKEN}"'

chunksize = 500000000
i=0
for x in range(0, numbytes, chunksize):
  if x+chunksize > numbytes:
    end=numbytes
  else:
    end=x-1+chunksize
  os.system("curl %s --range %d-%d -L %s -o %s.part%02d" % (tokenstr, x, end, url, prefix,i))
  i=i+1

