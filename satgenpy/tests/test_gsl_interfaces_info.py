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


class TestGslInterfacesInfo(unittest.TestCase):

    def test_simple(self):
        satgen.generate_simple_gsl_interfaces_info(
            "gsl_interfaces_info.txt.tmp",
            33,
            55,
            3,
            5,
            7.0,
            10.0
        )
        gsl_interfaces_info = satgen.read_gsl_interfaces_info("gsl_interfaces_info.txt.tmp", 33, 55)
        self.assertEqual(33 + 55, len(gsl_interfaces_info))
        for i in range(33 + 55):
            if i < 33:
                self.assertEqual(gsl_interfaces_info[i]["number_of_interfaces"], 3)
                self.assertEqual(gsl_interfaces_info[i]["aggregate_max_bandwidth"], 7.0)
            else:
                self.assertEqual(gsl_interfaces_info[i]["number_of_interfaces"], 5)
                self.assertEqual(gsl_interfaces_info[i]["aggregate_max_bandwidth"], 10.0)
            for k in gsl_interfaces_info[i].keys():
                self.assertTrue(k in ("number_of_interfaces", "aggregate_max_bandwidth"))
        os.remove("gsl_interfaces_info.txt.tmp")

    def test_read_more(self):
        with open("gsl_interfaces_info.txt.tmp", "w+") as f_out:
            f_out.write("0,5,10.0\n")
            f_out.write("1,7,9.0\n")
        gsl_interfaces_info = satgen.read_gsl_interfaces_info("gsl_interfaces_info.txt.tmp", 1, 1)
        self.assertEqual(2, len(gsl_interfaces_info))
        self.assertEqual(gsl_interfaces_info[0]["number_of_interfaces"], 5)
        self.assertEqual(gsl_interfaces_info[0]["aggregate_max_bandwidth"], 10.0)
        self.assertEqual(gsl_interfaces_info[1]["number_of_interfaces"], 7)
        self.assertEqual(gsl_interfaces_info[1]["aggregate_max_bandwidth"], 9.0)
        os.remove("gsl_interfaces_info.txt.tmp")

    def test_invalid(self):

        # Cannot have 0 GSL interfaces
        with open("gsl_interfaces_info.temp.txt", "w+") as f_out:
            f_out.write("0,0,2.0")
        try:
            satgen.read_gsl_interfaces_info("gsl_interfaces_info.temp.txt", 1, 0)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("gsl_interfaces_info.temp.txt")

        # Amount don't match
        with open("gsl_interfaces_info.temp.txt", "w+") as f_out:
            f_out.write("0,5,10.0\n")
            f_out.write("1,7,9.0\n")
        try:
            satgen.read_gsl_interfaces_info("gsl_interfaces_info.temp.txt", 1, 2)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("gsl_interfaces_info.temp.txt")

        # Node ID does not increment
        with open("gsl_interfaces_info.temp.txt", "w+") as f_out:
            f_out.write("1,5,10.0\n")
            f_out.write("2,7,9.0\n")
        try:
            satgen.read_gsl_interfaces_info("gsl_interfaces_info.temp.txt", 1, 1)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("gsl_interfaces_info.temp.txt")

        # Incorrect number of columns
        with open("gsl_interfaces_info.temp.txt", "w+") as f_out:
            f_out.write("0,5,10.0,2\n")
            f_out.write("1,7,9.0\n")
        try:
            satgen.read_gsl_interfaces_info("gsl_interfaces_info.temp.txt", 1, 1)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("gsl_interfaces_info.temp.txt")

        # Cannot have negative bandwidth
        with open("gsl_interfaces_info.temp.txt", "w+") as f_out:
            f_out.write("0,1,-2.0")
        try:
            satgen.read_gsl_interfaces_info("gsl_interfaces_info.temp.txt", 1, 0)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("gsl_interfaces_info.temp.txt")

        # Cannot have zero bandwidth
        with open("gsl_interfaces_info.temp.txt", "w+") as f_out:
            f_out.write("0,1,0.0")
        try:
            satgen.read_gsl_interfaces_info("gsl_interfaces_info.temp.txt", 1, 0)
            self.fail()
        except ValueError:
            self.assertTrue(True)
        os.remove("gsl_interfaces_info.temp.txt")
