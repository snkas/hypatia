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


class TestDescription(unittest.TestCase):

    def test_description(self):
        satgen.generate_description("description.txt.tmp", 19229.242421942, 1828282.2290193001)
        with open("description.txt.tmp", "r") as f_in:
            i = 0
            for line in f_in:
                if i == 0:
                    self.assertEqual("max_gsl_length_m=19229.2424219420", line.strip())
                elif i == 1:
                    self.assertEqual("max_isl_length_m=1828282.2290193001", line.strip())
                else:
                    self.fail()
                i += 1
        os.remove("description.txt.tmp")
