# The MIT License (MIT)
#
# Copyright (c) 2020 ETH Zurich
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys
from satgen.post_analysis.print_graphical_routes_and_rtt import print_graphical_routes_and_rtt


def main():
    args = sys.argv[1:]
    if len(args) != 6:
        print("Must supply exactly six arguments")
        print("Usage: python -m satgen.post_analysis.main_print_graphical_routes_and_rtt.py [data_dir] "
              "[satellite_network_dir] [dynamic_state_update_interval_ms] [end_time_s] [src] [dst]")
        exit(1)
    else:
        core_network_folder_name = args[1].split("/")[-1]
        base_output_dir = "%s/%s/%dms_for_%ds/manual" % (
            args[0], core_network_folder_name, int(args[2]), int(args[3])
        )
        print("Data dir: " + args[0])
        print("Used data dir to form base output dir: " + base_output_dir)
        print_graphical_routes_and_rtt(
            base_output_dir,
            args[1],
            int(args[2]),
            int(args[3]),
            int(args[4]),
            int(args[5])
        )


if __name__ == "__main__":
    main()
