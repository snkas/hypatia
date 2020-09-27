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
from satgen.post_analysis.analyze_time_step_path import analyze_time_step_path


def main():
    args = sys.argv[1:]
    if len(args) != 4:
        print("Must supply exactly four arguments")
        print("Usage: python  -m satgen.post_analysis.analyze_step_changes.py [output_data_dir] "
              "[satellite_network_dir] [dynamic_state_update_interval_ms: e.g., 100,1000] [end_time_s]")
        exit(1)
    else:
        analyze_time_step_path(
            args[0],
            args[1],
            list(map(lambda x: int(x), args[2].split(","))),
            int(args[3])
        )


if __name__ == "__main__":
    main()
