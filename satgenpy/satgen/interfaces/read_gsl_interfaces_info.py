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

import exputil


def read_gsl_interfaces_info(filename_gsl_interfaces_info, number_of_satellites, number_of_ground_stations):
    """
    Read for each node the GSL interface information

    :param filename_gsl_interfaces_info: Filename of GSL interfaces info file
                                         (typically /path/to/gsl_interfaces_info.txt)
                                         Line format: <node id>,<number of interfaces>,<aggregate max. bandwidth
                                         (unit specified but must be the same for all)>
    :param number_of_satellites:         Number of satellites (for check)
    :param number_of_ground_stations:    Number of ground stations (for check)

    :return: GSL interface information list of objects with format: {
        "number_of_interfaces" : <integer>
        "aggregate_max_bandwidth" : <aggregate_max_bandwidth> (there is no unit)
    }
    """
    list_gsl_interfaces_info = []
    node_id = 0
    with open(filename_gsl_interfaces_info, 'r') as f:
        for line in f:
            split = line.split(',')

            if len(split) != 3:
                raise ValueError("Must have three columns: "
                                 "<node id>,<number of interfaces>,<aggregate max. bandwidth>")

            if int(split[0]) != node_id:
                raise ValueError("Node id must increment each line")

            num_interfaces = exputil.parse_positive_int(split[1])
            if num_interfaces == 0:
                raise ValueError("Node must have at least one interface")

            aggregate_max_bandwidth = exputil.parse_positive_float(split[2])
            if aggregate_max_bandwidth == 0:
                raise ValueError("Aggregate max. bandwidth cannot be zero")

            list_gsl_interfaces_info.append({
                "number_of_interfaces": num_interfaces,
                "aggregate_max_bandwidth": aggregate_max_bandwidth
            })

            node_id += 1
        if node_id != number_of_satellites + number_of_ground_stations:
            raise ValueError("Number of nodes defined does not match up with number of satellites and ground stations")
    return list_gsl_interfaces_info
