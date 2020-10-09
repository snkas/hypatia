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

from satgen.distance_tools import *
from .read_ground_stations import *


def extend_ground_stations(filename_ground_stations_basic_in, filename_ground_stations_out):
    ground_stations = read_ground_stations_basic(filename_ground_stations_basic_in)
    with open(filename_ground_stations_out, "w+") as f_out:
        for ground_station in ground_stations:
            cartesian = geodetic2cartesian(
                float(ground_station["latitude_degrees_str"]),
                float(ground_station["longitude_degrees_str"]),
                ground_station["elevation_m_float"]
            )
            f_out.write(
                "%d,%s,%f,%f,%f,%f,%f,%f\n" % (
                    ground_station["gid"],
                    ground_station["name"],
                    float(ground_station["latitude_degrees_str"]),
                    float(ground_station["longitude_degrees_str"]),
                    ground_station["elevation_m_float"],
                    cartesian[0],
                    cartesian[1],
                    cartesian[2]
                )
            )
