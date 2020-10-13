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
import exputil


class TestIsls(unittest.TestCase):

    def test_isls_empty(self):
        satgen.generate_empty_isls("isls_empty.txt.tmp")
        isls_list = satgen.read_isls("isls_empty.txt.tmp", 0)
        self.assertEqual(0, len(isls_list))
        os.remove("isls_empty.txt.tmp")

    def test_isls_plus_grid(self):
        for values in [(25, 25, 1), (10, 5, 0), (24, 66, 6), (3, 3, 0)]:
            num_orbits = values[0]
            num_sat_per_orbit = values[1]
            isl_shift = values[2]
            satgen.generate_plus_grid_isls("isls.txt.tmp", num_orbits, num_sat_per_orbit, isl_shift)
            isls_list = satgen.read_isls("isls.txt.tmp", num_orbits * num_sat_per_orbit)
            os.remove("isls.txt.tmp")
            self.assertEqual(len(isls_list), num_orbits * num_sat_per_orbit * 2)
            self.assertEqual(len(set(isls_list)), num_orbits * num_sat_per_orbit * 2)
            for i in range(num_orbits * num_sat_per_orbit):
                orbit_of_i = int(floor(i / float(num_sat_per_orbit)))

                # Links in same orbit
                neighbor_1 = orbit_of_i * num_sat_per_orbit + (i + num_sat_per_orbit + 1) % num_sat_per_orbit
                neighbor_2 = orbit_of_i * num_sat_per_orbit + (i + num_sat_per_orbit - 1) % num_sat_per_orbit

                # Links to different orbits
                neighbor_3 = (
                    ((orbit_of_i + num_orbits - 1) % num_orbits)
                    * num_sat_per_orbit + (i + num_sat_per_orbit - isl_shift) % num_sat_per_orbit
                )
                neighbor_4 = (
                    ((orbit_of_i + num_orbits + 1) % num_orbits)
                    * num_sat_per_orbit + (i + num_sat_per_orbit + isl_shift) % num_sat_per_orbit
                )

                # All of them must be present
                self.assertTrue((min(i, neighbor_1), max(i, neighbor_1)) in isls_list)
                self.assertTrue((min(i, neighbor_2), max(i, neighbor_2)) in isls_list)
                self.assertTrue((min(i, neighbor_3), max(i, neighbor_3)) in isls_list)
                self.assertTrue((min(i, neighbor_4), max(i, neighbor_4)) in isls_list)

    def test_isls_plus_grid_invalid(self):
        try:
            satgen.generate_plus_grid_isls("isls.txt.tmp", 2, 2, 0)
            self.fail()
        except ValueError:
            self.assertTrue(True)

        try:
            satgen.generate_plus_grid_isls("isls.txt.tmp", 3, 2, 0)
            self.fail()
        except ValueError:
            self.assertTrue(True)

        try:
            satgen.generate_plus_grid_isls("isls.txt.tmp", 1, 1, 0)
            self.fail()
        except ValueError:
            self.assertTrue(True)

    def test_isls_invalid_file(self):
        local_shell = exputil.LocalShell()

        # Invalid left index
        local_shell.write_file(
            "isls.txt.tmp",
            "2 3\n5 6\n9 0"
        )
        try:
            satgen.read_isls("isls.txt.tmp", 9)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("isls.txt.tmp")

        # Invalid right index
        local_shell.write_file(
            "isls.txt.tmp",
            "2 3\n5 6\n6 9\n3 99"
        )
        try:
            satgen.read_isls("isls.txt.tmp", 50)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("isls.txt.tmp")

        # Invalid left index
        local_shell.write_file(
            "isls.txt.tmp",
            "2 3\n5 6\n6 8\n-3 3"
        )
        try:
            satgen.read_isls("isls.txt.tmp", 50)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("isls.txt.tmp")

        # Invalid right index
        local_shell.write_file(
            "isls.txt.tmp",
            "2 3\n5 6\n1 -3\n6 8"
        )
        try:
            satgen.read_isls("isls.txt.tmp", 50)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("isls.txt.tmp")

        # Left is larger than right
        local_shell.write_file(
            "isls.txt.tmp",
            "6 5"
        )
        try:
            satgen.read_isls("isls.txt.tmp", 10)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("isls.txt.tmp")

        # Left is equal to right
        local_shell.write_file(
            "isls.txt.tmp",
            "5 5"
        )
        try:
            satgen.read_isls("isls.txt.tmp", 10)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("isls.txt.tmp")

        # Duplicate
        local_shell.write_file(
            "isls.txt.tmp",
            "2 3\n5 6\n3 9\n5 6\n2 9"
        )
        try:
            satgen.read_isls("isls.txt.tmp", 10)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("isls.txt.tmp")
