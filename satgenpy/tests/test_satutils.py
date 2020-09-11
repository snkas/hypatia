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

import satgen
import unittest
from math import floor
import os


class TestSatUtils(unittest.TestCase):

    # def test_read_ground_stations(self):
    #     result = satgen.read_ground_stations_extended("data/legacy/ground_stations_first_100.txt")
    #     self.assertEqual(len(result), 100)
    #
    # def test_read_tles(self):
    #     result = satgen.read_tles("data/legacy/starlink_tles_25x25.txt")
    #     self.assertEqual(result["n_orbits"], 25)
    #     self.assertEqual(result["n_sats_per_orbit"], 25)
    #     self.assertEqual(len(result["satellites"]), 625)

    def test_isls(self):
        for values in [(25, 25, 1), (10, 5, 0), (24, 66, 6), (3, 3, 0)]:
            num_orbits = values[0]
            num_sat_per_orbit = values[1]
            isl_shift = values[2]
            satgen.generate_plus_grid_isls("isls.txt.tmp", num_orbits, num_sat_per_orbit, isl_shift)
            isls_list = satgen.read_isls("isls.txt.tmp")
            os.remove("isls.txt.tmp")
            self.assertEqual(len(isls_list), num_orbits * num_sat_per_orbit * 2)
            self.assertEqual(len(set(isls_list)), num_orbits * num_sat_per_orbit * 2)
            for i in range(num_orbits * num_sat_per_orbit):
                orbit_of_i = int(floor(i / float(num_sat_per_orbit)))

                # Links in same orbit
                neighbor_1 = orbit_of_i * num_sat_per_orbit + (i + num_sat_per_orbit + 1) % num_sat_per_orbit
                neighbor_2 = orbit_of_i * num_sat_per_orbit + (i + num_sat_per_orbit - 1) % num_sat_per_orbit

                # Links to different orbits
                neighbor_3 = ((orbit_of_i + num_orbits - 1) % num_orbits) * num_sat_per_orbit + (i + num_sat_per_orbit - isl_shift) % num_sat_per_orbit
                neighbor_4 = ((orbit_of_i + num_orbits + 1) % num_orbits) * num_sat_per_orbit + (i + num_sat_per_orbit + isl_shift) % num_sat_per_orbit

                # All of them must be present
                self.assertTrue((min(i, neighbor_1), max(i, neighbor_1)) in isls_list)
                self.assertTrue((min(i, neighbor_2), max(i, neighbor_2)) in isls_list)
                self.assertTrue((min(i, neighbor_3), max(i, neighbor_3)) in isls_list)
                self.assertTrue((min(i, neighbor_4), max(i, neighbor_4)) in isls_list)
