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

from exputil import parse_positive_int


def read_isls(filename_isls, num_satellites):
    """
    Read ISLs file into a list of undirected edges

    :param filename_isls:  Filename of ISLs (typically /path/to/isls.txt)
    :param num_satellites: Number of satellites (to verify indices)

    :return: List of all undirected ISL edges
    """
    isls_list = []
    with open(filename_isls, 'r') as f:
        isls_set = set()
        for line in f:
            line_spl = line.split()
            a = parse_positive_int(line_spl[0])
            b = parse_positive_int(line_spl[1])

            # Verify the input
            if a >= num_satellites:
                raise ValueError("Satellite does not exist: %d" % a)
            if b >= num_satellites:
                raise ValueError("Satellite does not exist: %d" % b)
            if b <= a:
                raise ValueError("The second satellite index must be strictly larger than the first")
            if (a, b) in isls_set:
                raise ValueError("Duplicate ISL: (%d, %d)" % (a, b))
            isls_set.add((a, b))

            # Finally add it to the list
            isls_list.append((a, b))

    return isls_list
