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
import unittest
from satgen import *


class TestDynamicState(unittest.TestCase):

    def test_around_equator_connectivity_with_starlink(self):
        local_shell = exputil.LocalShell()

        # Output directory
        temp_gen_data = "temp_dynamic_state_gen_data"
        name = "small_equator_constellation"
        local_shell.make_full_dir(temp_gen_data + "/" + name)

        # At t=0
        # epoch = Time("2000-01-01 00:00:00", scale="tdb")
        # time_since_epoch_ns = 0
        time_step_ms = 1000
        duration_s = 1

        # Ground stations
        local_shell.write_file(
            temp_gen_data + "/" + name + "/ground_stations.txt",
            (
                "0,Luanda,-8.836820,13.234320,0.000000,6135530.183815,1442953.502786,-973332.344974\n"
                "1,Lagos,6.453060,3.395830,0.000000,6326864.177950,375422.898833,712064.787620\n"
                "2,Kinshasa,-4.327580,15.313570,0.000000,6134256.671861,1679704.404461,-478073.165313\n"
                "3,Ar-Riyadh-(Riyadh),24.690466,46.709566,0.000000,3975957.341095,4220595.030186,2647959.980346"
            )
        )

        # Satellites (TLEs)
        local_shell.write_file(
            temp_gen_data + "/" + name + "/tles.txt",
            (
                "1 4\n"
                "Starlink-550 0\n"
                "1 01308U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05\n"
                "2 01308  53.0000 295.0000 0000001   0.0000 155.4545 15.19000000    04\n"
                "Starlink-550 1\n"
                "1 01309U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    06\n"
                "2 01309  53.0000 295.0000 0000001   0.0000 171.8182 15.19000000    04\n"
                "Starlink-550 2\n"
                "1 01310U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    08\n"
                "2 01310  53.0000 295.0000 0000001   0.0000 188.1818 15.19000000    03\n"
                "Starlink-550 3\n"
                "1 01311U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    09\n"
                "2 01311  53.0000 295.0000 0000001   0.0000 204.5455 15.19000000    04"
            )
        )

        # ISLs
        local_shell.write_file(
            temp_gen_data + "/" + name + "/isls.txt",
            (
                "0 1\n"
                "1 2\n"
                "2 3"
            )
        )

        # GSL interfaces info
        local_shell.write_file(
            temp_gen_data + "/" + name + "/gsl_interfaces_info.txt",
            (
                "0,1,1.0\n"
                "1,1,1.0\n"
                "2,1,1.0\n"
                "3,1,1.0\n"
                "4,1,1.0\n"
                "5,1,1.0\n"
                "6,1,1.0\n"
                "7,1,1.0"
            )
        )

        # Maximum GSL / ISL length
        max_gsl_length_m = 1089686.4181956202
        max_isl_length_m = 5016591.2330984278

        # Algorithm
        dynamic_state_algorithm = "algorithm_free_one_only_over_isls"

        # Call the helper
        help_dynamic_state(
            temp_gen_data,
            1,
            name,
            time_step_ms,
            duration_s,
            max_gsl_length_m,
            max_isl_length_m,
            dynamic_state_algorithm,
            True
        )

        # Now we are going to compare the generated fstate_0.txt and gsl_if_bandwidth_0.txt
        # again what is the expected outcome.

        # Forwarding state
        fstate = {}
        with open(temp_gen_data + "/" + name + "/dynamic_state_1000ms_for_1s/fstate_0.txt", "r") as f_in:
            for line in f_in:
                spl = line.split(",")
                self.assertEqual(len(spl), 5)
                fstate[(int(spl[0]), int(spl[1]))] = (int(spl[2]), int(spl[3]), int(spl[4]))

        # Check forwarding state content
        self.assertEqual(len(fstate.keys()), 8 * 4 - 4)

        # Satellite 0 always forwards to satellite 1 as it is out of range of all others
        self.assertEqual(fstate[(0, 4)], (1, 0, 0))
        self.assertEqual(fstate[(0, 5)], (1, 0, 0))
        self.assertEqual(fstate[(0, 6)], (1, 0, 0))
        self.assertEqual(fstate[(0, 7)], (-1, -1, -1))

        # Satellite 1 has Lagos (5) in range, but the others not
        self.assertEqual(fstate[(1, 4)], (2, 1, 0))
        self.assertEqual(fstate[(1, 5)], (5, 2, 0))
        self.assertEqual(fstate[(1, 6)], (2, 1, 0))
        self.assertEqual(fstate[(1, 7)], (-1, -1, -1))

        # Satellite 2 has (4, 6) in range, but the others not
        self.assertEqual(fstate[(2, 4)], (4, 2, 0))
        self.assertEqual(fstate[(2, 5)], (1, 0, 1))
        self.assertEqual(fstate[(2, 6)], (6, 2, 0))
        self.assertEqual(fstate[(2, 7)], (-1, -1, -1))

        # Satellite 3 has none in range
        self.assertEqual(fstate[(3, 4)], (2, 0, 1))
        self.assertEqual(fstate[(3, 5)], (2, 0, 1))
        self.assertEqual(fstate[(3, 6)], (2, 0, 1))
        self.assertEqual(fstate[(3, 7)], (-1, -1, -1))

        # Ground station 0 (id: 4) has satellite 2 in range
        self.assertEqual(fstate[(4, 5)], (2, 0, 2))
        self.assertEqual(fstate[(4, 6)], (2, 0, 2))
        self.assertEqual(fstate[(4, 7)], (-1, -1, -1))

        # Ground station 1 (id: 5) has satellite 1 in range
        self.assertEqual(fstate[(5, 4)], (1, 0, 2))
        self.assertEqual(fstate[(5, 6)], (1, 0, 2))
        self.assertEqual(fstate[(5, 7)], (-1, -1, -1))

        # Ground station 2 (id: 6) has satellite 2 in range
        self.assertEqual(fstate[(6, 4)], (2, 0, 2))
        self.assertEqual(fstate[(6, 5)], (2, 0, 2))
        self.assertEqual(fstate[(6, 7)], (-1, -1, -1))

        # Ground station 3 (id: 7) has no satellites in range
        self.assertEqual(fstate[(7, 4)], (-1, -1, -1))
        self.assertEqual(fstate[(7, 5)], (-1, -1, -1))
        self.assertEqual(fstate[(7, 6)], (-1, -1, -1))

        # GSL interface bandwidth
        gsl_if_bandwidth = {}
        with open(temp_gen_data + "/" + name + "/dynamic_state_1000ms_for_1s/gsl_if_bandwidth_0.txt", "r") as f_in:
            for line in f_in:
                spl = line.split(",")
                self.assertEqual(len(spl), 3)
                gsl_if_bandwidth[(int(spl[0]), int(spl[1]))] = float(spl[2])

        # Check GSL interface content
        self.assertEqual(len(gsl_if_bandwidth.keys()), 8)
        for node_id in range(8):
            if node_id == 1 or node_id == 2:
                self.assertEqual(gsl_if_bandwidth[(node_id, 2)], 1.0)
            elif node_id == 0 or node_id == 3:
                self.assertEqual(gsl_if_bandwidth[(node_id, 1)], 1.0)
            else:
                self.assertEqual(gsl_if_bandwidth[(node_id, 0)], 1.0)

        # Clean up
        local_shell.remove_force_recursive(temp_gen_data)
