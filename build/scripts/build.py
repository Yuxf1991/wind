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


import getopt
import os
import shutil
import sys

import cmd_utils
import format_codes


# Debug or Release
default_build_type = "Debug"


def print_helps():
    print("-h --help: print helps.")
    print("-j --jobs: jobs num to build project.")
    print("-b --build-type: build type, Debug or Release.")
    print("-f --format-codes: format oewm source codes according to .clang-format you defined in project root.")


def build_gn(gn_source_abs_path, gn_out_abs_path, job_nums):
    print("Entering %s" % gn_source_abs_path)
    os.chdir(gn_source_abs_path)

    if cmd_utils.execute("python build/gen.py") != 0:
        return -1

    if cmd_utils.execute("ninja -C out -j %d" % job_nums) != 0:
        return -1

    # copy gn binary
    if not os.path.exists(gn_out_abs_path):
        os.mkdir(gn_out_abs_path)
    shutil.copy("out/gn", os.path.join(gn_out_abs_path, "gn"))

    return 0


def build_wind(gn_abs_path, wind_project_path, wind_out_path, build_type, jobs_num):
    is_debug = "false"
    if build_type == "debug" or build_type == "Debug":
        is_debug = "true"

    # configure
    if cmd_utils.execute(
        "%s gen %s/ --args=\'is_debug=%s\' --export-compile-commands" %(gn_abs_path, wind_out_path, is_debug)) != 0:
        return -1

    # build
    ret = cmd_utils.execute("ninja -C %s -j %d" % (wind_out_path, jobs_num))
    if ret != 0:
        return ret

    return 0


def build_all(project_root_abs_path, build_type, jobs_num):
    out_path = os.path.join(project_root_abs_path, "out", build_type)
    wind_codes_directory = os.path.join(project_root_abs_path, "wind")
    wind_out_path = os.path.join(out_path, "wind")
    samples_codes_directory = os.path.join(project_root_abs_path, "samples")
    samples_out_path = os.path.join(out_path, "samples")

    thirdparty_path = os.path.join(project_root_abs_path, "thirdparty")
    # build gn
    out_gn_path = os.path.join(out_path, "gn")
    if not os.path.exists(out_gn_path):
        os.mkdir(out_gn_path)
    gn_sources_path = os.path.join(thirdparty_path, "gn")
    if build_gn(gn_sources_path, out_gn_path, jobs_num) != 0:
        return -1

    gn_binary_path = os.path.join(out_gn_path, "gn")
    # build wind project
    if build_wind(gn_binary_path, wind_codes_directory, wind_out_path, build_type, jobs_num) != 0:
        return -1

    return 0


def main(argv):
    opts, _ = getopt.getopt(argv[1:], "-h-j:-b:-f", ["help", "jobs=", "build-type=", "format-codes"])
    jobs_num = os.cpu_count()
    build_type = default_build_type
    only_format = False
    for (opt_name, opt_val) in opts:
        if opt_name in ("-h", "--help"):
            print_helps()
            return
        if opt_name in ("-j", "--jobs"):
            jobs_num = int(opt_val)
        if opt_name in ("-b", "--build_type"):
            build_type = opt_val.capitalize()
            if build_type not in ["Debug", "Release"]:
                print("build_type %s is not correct, should be Debug or Release." % build_type)
                return
        if opt_name in ("-f", "--format_codes"):
            only_format = True

    this_script_abs_path = os.path.dirname(__file__)
    project_root_abs_path = os.path.abspath(os.path.join(this_script_abs_path, "..", ".."))

    # format sources only
    if only_format == True:
        wind_codes_directory = os.path.join(project_root_abs_path, "wind")
        samples_codes_directory = os.path.join(project_root_abs_path, "samples")
        format_codes.format(wind_codes_directory)
        format_codes.format(samples_codes_directory)
        return

    if build_all(project_root_abs_path, build_type, jobs_num) != 0:
        print("Build failed.")
    else:
        print("Build done.")


if __name__ == "__main__":
    main(sys.argv)
