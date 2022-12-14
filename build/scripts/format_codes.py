# MIT License

# Copyright (c) 2022 Tracy

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#!/usr/bin/python


import os
import sys

import cmd_utils


to_format_exts = [".c", ".cc", ".cxx", ".cpp", ".h", ".hpp"]


def format(source_code_directory):
    format_cmd = "clang-format -style=file -i "
    print("format codes for %s start." % source_code_directory)
    source_code_abspath = os.path.abspath(source_code_directory)
    for [dirpath, _, filenames] in os.walk(source_code_abspath):
        cur_abs_path = os.path.join(source_code_abspath, dirpath)
        for filename in filenames:
            abs_file_name = os.path.join(cur_abs_path, filename)
            need_to_format = False
            for ext in to_format_exts:
                if abs_file_name.endswith(ext):
                    need_to_format = True
                    break
            if need_to_format:
                format_cmd_tmp = format_cmd + abs_file_name
                if cmd_utils.execute(format_cmd_tmp) != 0:
                    sys.exit("[!] exit unexpectly.")
    print("format codes for %s done." % source_code_directory)
