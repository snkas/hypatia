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


# # import exputil
# # from astropy.time import Time
# # from astropy import units as u
# # from satgen import read_ground_stations_basic
#
# import unittest
# import ephem
# import math
# from satgen.distance_tools import *
# from astropy.time import Time
# from astropy import units as u
#
#
# class TestDynamicState(unittest.TestCase):
#
# #     def test_one_sat_two_gs(self):
# #         local_shell = exputil.LocalShell()
# #
# #         # Output directory
# #         temp_dir = "temp_fstate_calculation_test"
# #         local_shell.make_full_dir(temp_dir)
# #         output_dynamic_state_dir = temp_dir
# #
# #         epoch = Time("2000-01-01 00:00:00", scale="tdb")
# #         time_since_epoch_ns = 0
# #         satellites = [
# #
# #         ]
# #         with open(output_dynamic_state_dir + "/ground_stations.txt", "w+") as f_out:
# #             f_out.write("gid,name,latitude,longitude.elevation\n")
# #
# #
# #         list_isls = [
# #
# #         ]
# #
# #         list_gsl_interfaces_info = [
# #
# #         ]
# #         max_gsl_length_m = 10000000000
# #         max_isl_length_m = 10000000000
# #         dynamic_state_algorithm = "algorithm_paired_many_only_over_isls"
# #         prev_output = None
# #         enable_verbose_logs = True
# #
# #         generate_dynamic_state_at(
# #             output_dynamic_state_dir,
# #             epoch,
# #             time_since_epoch_ns,
# #             satellites,
# #             ground_stations,
# #             list_isls,
# #             list_gsl_interfaces_info,
# #             max_gsl_length_m,
# #             max_isl_length_m,
# #             dynamic_state_algorithm,
# #             prev_output,
# #             enable_verbose_logs
# #         )
