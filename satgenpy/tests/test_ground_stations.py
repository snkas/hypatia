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
import os


class TestGroundStations(unittest.TestCase):

    def test_ground_stations_normal(self):

        # Write basic ground stations
        with open("ground_stations.temp.txt", "w+") as f_out:
            f_out.write("0,abc,33,11.0,77")

        # Read basic and compare
        ground_stations = satgen.read_ground_stations_basic("ground_stations.temp.txt")
        self.assertEqual(1, len(ground_stations))
        self.assertEqual(0, ground_stations[0]["gid"])
        self.assertEqual("abc", ground_stations[0]["name"])
        self.assertEqual("33", ground_stations[0]["latitude_degrees_str"])
        self.assertEqual("11.0", ground_stations[0]["longitude_degrees_str"])
        self.assertEqual(77.0, ground_stations[0]["elevation_m_float"])
        self.assertTrue("cartesian_x" not in ground_stations[0])
        self.assertTrue("cartesian_y" not in ground_stations[0])
        self.assertTrue("cartesian_z" not in ground_stations[0])

        # Extend
        satgen.extend_ground_stations("ground_stations.temp.txt", "ground_stations_extended.temp.txt")
        ground_stations_extended = satgen.read_ground_stations_extended("ground_stations_extended.temp.txt")
        self.assertEqual(1, len(ground_stations_extended))
        self.assertEqual(0, ground_stations_extended[0]["gid"])
        self.assertEqual("abc", ground_stations_extended[0]["name"])
        self.assertEqual("33.000000", ground_stations_extended[0]["latitude_degrees_str"])
        self.assertEqual("11.000000", ground_stations_extended[0]["longitude_degrees_str"])
        self.assertEqual(77.0, ground_stations_extended[0]["elevation_m_float"])
        self.assertTrue("cartesian_x" in ground_stations_extended[0])
        self.assertTrue("cartesian_y" in ground_stations_extended[0])
        self.assertTrue("cartesian_z" in ground_stations_extended[0])
        # TODO: Verify Cartesian coordinates

        # Clean up
        os.remove("ground_stations.temp.txt")
        os.remove("ground_stations_extended.temp.txt")

    def test_ground_stations_valid(self):

        # Empty
        with open("ground_stations.temp.txt", "w+") as f_out:
            f_out.write("")
        self.assertEqual(0, len(satgen.read_ground_stations_basic("ground_stations.temp.txt")))
        os.remove("ground_stations.temp.txt")

        # Two lines
        with open("ground_stations.temp.txt", "w+") as f_out:
            f_out.write("0,abc,33,11,5\n")
            f_out.write("1,abc,33,11,5")
        self.assertEqual(2, len(satgen.read_ground_stations_basic("ground_stations.temp.txt")))
        os.remove("ground_stations.temp.txt")

    def test_ground_stations_invalid(self):

        # Missing column
        with open("ground_stations.temp.txt", "w+") as f_out:
            f_out.write("0,abc,33,11")
        try:
            satgen.read_ground_stations_basic("ground_stations.temp.txt")
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("ground_stations.temp.txt")

        # Invalid non-ascending gid
        with open("ground_stations.temp.txt", "w+") as f_out:
            f_out.write("0,abc,33,11,5\n")
            f_out.write("0,abc,33,11,5")
        try:
            satgen.read_ground_stations_basic("ground_stations.temp.txt")
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("ground_stations.temp.txt")

        # Missing column
        with open("ground_stations_extended.temp.txt", "w+") as f_out:
            f_out.write("0,abc,33,11,2,3,3")
        try:
            satgen.read_ground_stations_extended("ground_stations_extended.temp.txt")
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("ground_stations_extended.temp.txt")

        # Invalid non-ascending gid
        with open("ground_stations_extended.temp.txt", "w+") as f_out:
            f_out.write("0,abc,33,11,5,2,3,3\n")
            f_out.write("0,abc,33,11,5,2,3,3")
        try:
            satgen.read_ground_stations_extended("ground_stations_extended.temp.txt")
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("ground_stations_extended.temp.txt")
