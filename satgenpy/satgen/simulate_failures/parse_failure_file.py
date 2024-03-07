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

def parse_failure_file(failure_file):
    failure_table = {'SAT': {}, 'ISL': {}, 'GS': {}}
    with open(failure_file, "r") as f:
        for line in f:
            parts = line.strip().split(",")
            device = parts[0]
            if device == 'SAT' or device == 'GS':
                node_id, failure_start_time, failure_end_time = parts[1:]
                failure_table[device][int(node_id)] = (int(float(failure_start_time) * 1_000_000_000), int(float(failure_end_time) * 1_000_000_000))
            elif device == 'ISL':
                sat1, sat2, failure_start_time, failure_end_time = parts[1:]
                failure_table[device][(int(sat1), int(sat2))] = (int(float(failure_start_time) * 1_000_000_000), int(float(failure_end_time) * 1_000_000_000))
    return failure_table
