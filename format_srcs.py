# MIT License

# Copyright (c) 2020 Tracy

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

# format all c/c++ source files in directory wind by using clang-format
import os

format_cmd = "clang-format -i "
to_format_exts = [".c", ".cc", ".cxx", ".cpp", ".h", ".hpp"]

def format_srcs(abs_src_path):
    for [dirpath, dirnames, filenames] in os.walk(abs_src_path):
        cur_abs_path = os.path.join(abs_src_path, dirpath)
        for filename in filenames:
            abs_file_name = os.path.join(cur_abs_path, filename)
            to_format_flag = False
            for ext in to_format_exts:
                if abs_file_name.endswith(ext):
                    to_format_flag = True
                    break
            if to_format_flag:
                format_cmd_tmp = format_cmd + abs_file_name
                print(format_cmd_tmp)
                os.system(format_cmd_tmp)
                print(format_cmd_tmp + " done.")

if __name__ == "__main__":
    cur_abs_path = os.path.abspath(os.path.dirname(__file__))
    format_srcs(os.path.join(cur_abs_path, "wind"))
