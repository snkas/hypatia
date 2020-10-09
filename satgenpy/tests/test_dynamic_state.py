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

import unittest

# import exputil
# from astropy.time import Time
# from astropy import units as u
# from satgen import read_ground_stations_basic

from satgen.dynamic_state.generate_dynamic_state import *
from astropy.time import Time
from astropy import units as u

class TestDynamicState(unittest.TestCase):

    def test_sat_distance(self):
        kuiper_satellite_0 = ephem.readtle(
            "Kuiper-630 0",
            "1 00001U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    04",
            "2 00001  51.9000   0.0000 0000001   0.0000   0.0000 14.80000000    02"
        )

        kuiper_satellite_1 = ephem.readtle(
            "Kuiper-630 1",
            "1 00002U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05",
            "2 00002  51.9000   0.0000 0000001   0.0000  10.5882 14.80000000    07"
        )

        kuiper_satellite_17 = ephem.readtle(
            "Kuiper-630 17",
            "1 00018U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    02",
            "2 00018  51.9000   0.0000 0000001   0.0000 180.0000 14.80000000    09"
        )

        kuiper_satellite_18 = ephem.readtle(
            "Kuiper-630 18",
            "1 00019U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    03",
            "2 00019  51.9000   0.0000 0000001   0.0000 190.5882 14.80000000    04"
        )

        kuiper_satellite_19 = ephem.readtle(
            "Kuiper-630 19",
            "1 00020U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05",
            "2 00020  51.9000   0.0000 0000001   0.0000 201.1765 14.80000000    05"
        )

        for extra_time_ns in [
            0,  # 0
            1,  # 1ns
            1000,  # 1 microsecond
            1000000,  # 1ms
            1000000000,  # 1s
            60000000000,  # 60s
            10 * 60000000000,  # 10 minutes
            20 * 60000000000,  # 20 minutes
            30 * 60000000000,  # 30 minutes
            40 * 60000000000,  # 40 minutes
            50 * 60000000000,  # 50 minutes
            60 * 60000000000,  # 60 minutes
            70 * 60000000000,  # 70 minutes
            80 * 60000000000,  # 80 minutes
            90 * 60000000000,  # 90 minutes
            100 * 60000000000,  # 100 minutes
        ]:
            time = Time("2000-01-01 00:00:00", scale="tdb") + extra_time_ns * u.ns

            # Distance to themselves should always be zero
            self.assertEqual(sat_distance(kuiper_satellite_0, kuiper_satellite_0, str(time)), 0)
            self.assertEqual(sat_distance(kuiper_satellite_1, kuiper_satellite_1, str(time)), 0)
            self.assertEqual(sat_distance(kuiper_satellite_17, kuiper_satellite_17, str(time)), 0)
            self.assertEqual(sat_distance(kuiper_satellite_18, kuiper_satellite_18, str(time)), 0)

            # Distances should not matter if (a, b) or (b, a)
            self.assertEqual(
                sat_distance(kuiper_satellite_0, kuiper_satellite_1, str(time)),
                sat_distance(kuiper_satellite_1, kuiper_satellite_0, str(time)),
            )
            self.assertEqual(
                sat_distance(kuiper_satellite_1, kuiper_satellite_17, str(time)),
                sat_distance(kuiper_satellite_17, kuiper_satellite_1, str(time)),
            )
            self.assertEqual(
                sat_distance(kuiper_satellite_19, kuiper_satellite_17, str(time)),
                sat_distance(kuiper_satellite_17, kuiper_satellite_19, str(time)),
            )

            # Distance between 0 and 1 should be less than between 0 and 18 (must be on other side of planet)
            self.assertGreater(
                sat_distance(kuiper_satellite_0, kuiper_satellite_18, str(time)),
                sat_distance(kuiper_satellite_0, kuiper_satellite_1, str(time)),
            )

            # Triangle inequality
            self.assertGreater(
                sat_distance(kuiper_satellite_17, kuiper_satellite_18, str(time))
                +
                sat_distance(kuiper_satellite_18, kuiper_satellite_19, str(time))
                ,
                sat_distance(kuiper_satellite_17, kuiper_satellite_19, str(time)),
            )

            # Earth radius = 6378135 m
            # Kuiper altitude = 630 km
            # So, the circle is 630000 + 6378135 = 7008135 m in radius
            # As such, with 34 satellites, the side of this 34-agon is:
            hexagon_side_m = 2 * (7008135.0 * math.sin(math.radians(360.0 / 33.0) / 2.0))
            self.assertTrue(
                hexagon_side_m >= sat_distance(kuiper_satellite_17, kuiper_satellite_18, str(time)) >= 0.9 * hexagon_side_m
            )
            self.assertTrue(
                hexagon_side_m >= sat_distance(kuiper_satellite_18, kuiper_satellite_19, str(time)) >= 0.9 * hexagon_side_m
            )
            self.assertTrue(
                hexagon_side_m >= sat_distance(kuiper_satellite_0, kuiper_satellite_1, str(time)) >= 0.9 * hexagon_side_m
            )


#     def test_one_sat_two_gs(self):
#         local_shell = exputil.LocalShell()
#
#         # Output directory
#         temp_dir = "temp_fstate_calculation_test"
#         local_shell.make_full_dir(temp_dir)
#         output_dynamic_state_dir = temp_dir
#
#         epoch = Time("2000-01-01 00:00:00", scale="tdb")
#         time_since_epoch_ns = 0
#         satellites = [
#
#         ]
#         with open(output_dynamic_state_dir + "/ground_stations.txt", "w+") as f_out:
#             f_out.write("gid,name,latitude,longitude.elevation\n")
#
#
#         list_isls = [
#
#         ]
#
#         list_gsl_interfaces_info = [
#
#         ]
#         max_gsl_length_m = 10000000000
#         max_isl_length_m = 10000000000
#         dynamic_state_algorithm = "algorithm_paired_many_only_over_isls"
#         prev_output = None
#         enable_verbose_logs = True
#
#         generate_dynamic_state_at(
#             output_dynamic_state_dir,
#             epoch,
#             time_since_epoch_ns,
#             satellites,
#             ground_stations,
#             list_isls,
#             list_gsl_interfaces_info,
#             max_gsl_length_m,
#             max_isl_length_m,
#             dynamic_state_algorithm,
#             prev_output,
#             enable_verbose_logs
#         )
