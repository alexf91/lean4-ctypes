#!/bin/bash
#
# Copyright 2023 Alexander Fasching
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Taken from https://stackoverflow.com/a/246128
SCRIPTDIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd "$SCRIPTDIR"/../src

for file in $(ls *.cpp *.hpp); do
    include-what-you-use \
        -I ~/.elan/toolchains/leanprover--lean4---nightly-2023-08-15/include \
        $file 2> /tmp/iwyu_ctypes.out
    iwyu-fix-includes --nocomments < /tmp/iwyu_ctypes.out
done
