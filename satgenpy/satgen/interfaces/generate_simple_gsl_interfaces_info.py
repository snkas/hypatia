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


def generate_simple_gsl_interfaces_info(filename_gsl_interfaces_info, number_of_satellites, number_of_ground_stations,
                                        num_gsl_interfaces_per_satellite, num_gsl_interfaces_per_ground_station,
                                        agg_max_bandwidth_satellite, agg_max_bandwidth_ground_station):
    """
    Read for each node the GSL interface information.

    Note: the unit of the aggregate max bandwidth per satellite is not known, but they both must be the same unit.

    :param filename_gsl_interfaces_info: Filename of GSL interfaces info file to write to
                                         (typically /path/to/gsl_interfaces_info.txt)
                                         Line format: <node id>,<number of interfaces>,<aggregate max. bandwidth Mbit/s>
    :param number_of_satellites:                    Number of satellites
    :param number_of_ground_stations:               Number of ground stations
    :param num_gsl_interfaces_per_satellite:        Number of GSL interfaces per satellite
    :param num_gsl_interfaces_per_ground_station:   Number of (GSL) interfaces per ground station
    :param agg_max_bandwidth_satellite:             Aggregate bandwidth of all interfaces on a satellite
    :param agg_max_bandwidth_ground_station:        Aggregate bandwidth of all interfaces on a ground station

    """
    with open(filename_gsl_interfaces_info, 'w+') as f:
        for node_id in range(number_of_satellites + number_of_ground_stations):
            f.write("%d,%d,%f\n" % (
                node_id,
                num_gsl_interfaces_per_satellite if node_id < number_of_satellites
                else num_gsl_interfaces_per_ground_station,
                agg_max_bandwidth_satellite if node_id < number_of_satellites
                else agg_max_bandwidth_ground_station,
            ))
