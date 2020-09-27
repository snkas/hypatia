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
import math
import os
import exputil


# WGS72 value; taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html
EARTH_RADIUS = 6378135.0

# Target altitude of ~630 km
ALTITUDE_M = 630000

# Considering an elevation angle of 30 degrees; possible values [1]: 20(min)/30/35/45
SATELLITE_CONE_RADIUS_M = ALTITUDE_M / math.tan(math.radians(30.0))

# Maximum GSL length
MAX_GSL_LENGTH_M = math.sqrt(math.pow(SATELLITE_CONE_RADIUS_M, 2) + math.pow(ALTITUDE_M, 2))

# ISLs are not allowed to dip below 80 km altitude in order to avoid weather conditions
MAX_ISL_LENGTH_M = 2 * math.sqrt(math.pow(EARTH_RADIUS + ALTITUDE_M, 2) - math.pow(EARTH_RADIUS + 80000, 2))


class TestEndToEnd(unittest.TestCase):

    def test_end_to_end(self):
        local_shell = exputil.LocalShell()

        # Clean slate start
        local_shell.remove_force_recursive("temp_gen_data")
        local_shell.make_full_dir("temp_gen_data")

        # Both dynamic state algorithms should yield the same path and RTT
        for dynamic_state_algorithm in [
            "algorithm_free_one_only_over_isls",
            "algorithm_free_gs_one_sat_many_only_over_isls"
        ]:

            # Specific outcomes
            output_generated_data_dir = "temp_gen_data"
            num_threads = 1
            default_time_step_ms = 100
            all_time_step_ms = [50, 100, 1000, 10000]
            duration_s = 200

            # Add base name to setting
            name = "reduced_kuiper_630_" + dynamic_state_algorithm

            # Path trace we base this test on:
            # 0,1173-184-183-217-1241
            # 18000000000,1173-218-217-1241
            # 27600000000,1173-648-649-650-616-1241
            # 74300000000,1173-218-217-216-250-1241
            # 125900000000,1173-647-648-649-650-616-1241
            # 128700000000,1173-647-648-649-615-1241

            # Create output directories
            if not os.path.isdir(output_generated_data_dir):
                os.makedirs(output_generated_data_dir)
            if not os.path.isdir(output_generated_data_dir + "/" + name):
                os.makedirs(output_generated_data_dir + "/" + name)

            # Ground stations
            print("Generating ground stations...")
            with open(output_generated_data_dir + "/" + name + "/ground_stations.basic.txt", "w+") as f_out:
                f_out.write("0,Manila,14.6042,120.9822,0\n")  # Originally no. 17
                f_out.write("1,Dalian,38.913811,121.602322,0\n")  # Originally no. 85
            satgen.extend_ground_stations(
                output_generated_data_dir + "/" + name + "/ground_stations.basic.txt",
                output_generated_data_dir + "/" + name + "/ground_stations.txt"
            )

            # TLEs (taken from Kuiper-610 first shell)
            print("Generating TLEs...")
            with open(output_generated_data_dir + "/" + name + "/tles.txt", "w+") as f_out:
                f_out.write("1 12\n")  # Pretend it's one orbit with 12 satellites
                f_out.write("Kuiper-630 0\n")  # 183
                f_out.write("1 00184U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    06\n")
                f_out.write("2 00184  51.9000  52.9412 0000001   0.0000 142.9412 14.80000000    00\n")
                f_out.write("Kuiper-630 1\n")  # 184
                f_out.write("1 00185U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    07\n")
                f_out.write("2 00185  51.9000  52.9412 0000001   0.0000 153.5294 14.80000000    07\n")
                f_out.write("Kuiper-630 2\n")  # 216
                f_out.write("1 00217U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    03\n")
                f_out.write("2 00217  51.9000  63.5294 0000001   0.0000 127.0588 14.80000000    01\n")
                f_out.write("Kuiper-630 3\n")  # 217
                f_out.write("1 00218U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    04\n")
                f_out.write("2 00218  51.9000  63.5294 0000001   0.0000 137.6471 14.80000000    00\n")
                f_out.write("Kuiper-630 4\n")  # 218
                f_out.write("1 00219U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05\n")
                f_out.write("2 00219  51.9000  63.5294 0000001   0.0000 148.2353 14.80000000    08\n")
                f_out.write("Kuiper-630 5\n")  # 250
                f_out.write("1 00251U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    01\n")
                f_out.write("2 00251  51.9000  74.1176 0000001   0.0000 132.3529 14.80000000    00\n")
                f_out.write("Kuiper-630 6\n")  # 615
                f_out.write("1 00616U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    06\n")
                f_out.write("2 00616  51.9000 190.5882 0000001   0.0000  31.7647 14.80000000    05\n")
                f_out.write("Kuiper-630 7\n")  # 616
                f_out.write("1 00617U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    07\n")
                f_out.write("2 00617  51.9000 190.5882 0000001   0.0000  42.3529 14.80000000    03\n")
                f_out.write("Kuiper-630 8\n")  # 647
                f_out.write("1 00648U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    01\n")
                f_out.write("2 00648  51.9000 201.1765 0000001   0.0000  15.8824 14.80000000    09\n")
                f_out.write("Kuiper-630 9\n")  # 648
                f_out.write("1 00649U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    02\n")
                f_out.write("2 00649  51.9000 201.1765 0000001   0.0000  26.4706 14.80000000    07\n")
                f_out.write("Kuiper-630 10\n")  # 649
                f_out.write("1 00650U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    04\n")
                f_out.write("2 00650  51.9000 201.1765 0000001   0.0000  37.0588 14.80000000    05\n")
                f_out.write("Kuiper-630 11\n")  # 650
                f_out.write("1 00651U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05\n")
                f_out.write("2 00651  51.9000 201.1765 0000001   0.0000  47.6471 14.80000000    04\n")

            # Nodes
            #
            # Original ID   Test ID
            # 183           0
            # 184           1
            # 216           2
            # 217           3
            # 218           4
            # 250           5
            # 615           6
            # 616           7
            # 647           8
            # 648           9
            # 649           10
            # 650           11
            #
            # ISLs
            #
            # Original      Test
            # 183-184       0-1
            # 183-217       0-3
            # 216-217       2-3
            # 216-250       2-5
            # 217-218       3-4
            # 615-649       6-10
            # 616-650       7-11
            # 647-648       8-9
            # 648-649       9-10
            # 649-650       10-11
            #
            # Necessary ISLs (above) inferred from trace:
            #
            # 0,1173-184-183-217-1241
            # 18000000000,1173-218-217-1241
            # 27600000000,1173-648-649-650-616-1241
            # 74300000000,1173-218-217-216-250-1241
            # 125900000000,1173-647-648-649-650-616-1241
            # 128700000000,1173-647-648-649-615-1241
            #
            print("Generating ISLs...")
            with open(output_generated_data_dir + "/" + name + "/isls.txt", "w+") as f_out:
                f_out.write("0 1\n")
                f_out.write("0 3\n")
                f_out.write("2 3\n")
                f_out.write("2 5\n")
                f_out.write("3 4\n")
                f_out.write("6 10\n")
                f_out.write("7 11\n")
                f_out.write("8 9\n")
                f_out.write("9 10\n")
                f_out.write("10 11\n")

            # Description
            print("Generating description...")
            satgen.generate_description(
                output_generated_data_dir + "/" + name + "/description.txt",
                MAX_GSL_LENGTH_M,
                MAX_ISL_LENGTH_M
            )

            # Extended ground stations
            ground_stations = satgen.read_ground_stations_extended(
                output_generated_data_dir + "/" + name + "/ground_stations.txt"
            )

            # GSL interfaces
            if dynamic_state_algorithm == "algorithm_free_one_only_over_isls":
                gsl_interfaces_per_satellite = 1
                gsl_satellite_max_agg_bandwidth = 1.0
            elif dynamic_state_algorithm == "algorithm_free_gs_one_sat_many_only_over_isls":
                gsl_interfaces_per_satellite = len(ground_stations)
                gsl_satellite_max_agg_bandwidth = len(ground_stations)
            else:
                raise ValueError("Unknown dynamic state algorithm: " + dynamic_state_algorithm)
            print("Generating GSL interfaces info..")
            satgen.generate_simple_gsl_interfaces_info(
                output_generated_data_dir + "/" + name + "/gsl_interfaces_info.txt",
                12,  # 12 satellites
                len(ground_stations),
                gsl_interfaces_per_satellite,  # GSL interfaces per satellite
                1,  # (GSL) Interfaces per ground station
                gsl_satellite_max_agg_bandwidth,  # Aggregate max. bandwidth satellite (unit unspecified)
                1   # Aggregate max. bandwidth ground station (same unspecified unit)
            )

            # Forwarding state
            for time_step_ms in all_time_step_ms:
                print("Generating forwarding state...")
                satgen.help_dynamic_state(
                    output_generated_data_dir,
                    num_threads,
                    name,
                    time_step_ms,
                    duration_s,
                    MAX_GSL_LENGTH_M,
                    MAX_ISL_LENGTH_M,
                    dynamic_state_algorithm,
                    False
                )

            # Clean slate start
            local_shell.remove_force_recursive("temp_analysis_data")
            local_shell.make_full_dir("temp_analysis_data")
            output_analysis_data_dir = "temp_analysis_data"
            satgen.post_analysis.print_routes_and_rtt(
                output_analysis_data_dir + "/" + name,
                output_generated_data_dir + "/" + name,
                default_time_step_ms,
                duration_s,
                12,
                13,
                ""
            )

            # Now, we just want to see that the output path matches
            with open(output_analysis_data_dir + "/" + name + "/data/networkx_path_12_to_13.txt", "r") as f_in:
                i = 0
                for line in f_in:
                    line = line.strip()
                    if i == 0:
                        self.assertEqual(line, "0,12-1-0-3-13")
                    elif i == 1:
                        self.assertEqual(line, "18000000000,12-4-3-13")
                    elif i == 2:
                        self.assertEqual(line, "27600000000,12-9-10-11-7-13")
                    elif i == 3:
                        self.assertEqual(line, "74300000000,12-4-3-2-5-13")
                    elif i == 4:
                        self.assertEqual(line, "125900000000,12-8-9-10-11-7-13")
                    elif i == 5:
                        self.assertEqual(line, "128700000000,12-8-9-10-6-13")
                    else:
                        self.fail()
                    i += 1

            # ... and the RTT
            with open(output_analysis_data_dir + "/" + name + "/data/networkx_rtt_12_to_13.txt", "r") as f_in1:
                with open("tests/data_to_match/kuiper_630/networkx_rtt_1173_to_1241.txt", "r") as f_in2:
                    lines1 = []
                    for line in f_in1:
                        lines1.append(line.strip())
                    lines2 = []
                    for line in f_in2:
                        lines2.append(line.strip())

                    # Too computationally costly, so the below is equivalent: self.assertEqual(lines1, lines2)
                    self.assertEqual(len(lines1), len(lines2))
                    for i in range(len(lines1)):
                        a_spl = lines1[i].split(",")
                        b_spl = lines2[i].split(",")
                        self.assertEqual(len(a_spl), len(b_spl))
                        self.assertEqual(len(a_spl), 2)
                        a_time = int(a_spl[0])
                        b_time = int(b_spl[0])
                        a_rtt = float(a_spl[1])
                        b_rtt = float(b_spl[1])
                        self.assertEqual(a_time, b_time)
                        self.assertAlmostEqual(a_rtt, b_rtt, places=6)

            # Now let's run all analyses available

            # TODO: Disabled because it requires downloading files from CDNs, which can take too long
            # # Print graphically
            #
            # satgen.post_analysis.print_graphical_routes_and_rtt(
            #     output_analysis_data_dir + "/" + name,
            #     output_generated_data_dir + "/" + name,
            #     default_time_step_ms,
            #     duration_s,
            #     12,
            #     13
            # )

            # Analyze paths
            satgen.post_analysis.analyze_path(
                output_analysis_data_dir + "/" + name,
                output_generated_data_dir + "/" + name,
                default_time_step_ms,
                duration_s,
                ""
            )

            # Number of path changes per pair
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/path/data/ecdf_pairs_num_path_changes.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Only one pair with 5 path changes
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertEqual(columns[0][i], 5)

            # Max minus min hop count per pair
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/path/data/ecdf_pairs_max_minus_min_hop_count.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Shortest is 3 hops, longest is 6 hops, max delta is 3
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertEqual(columns[0][i], 3)

            # Max divided by min hop count per pair
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/path/data/ecdf_pairs_max_hop_count_to_min_hop_count.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Shortest is 3 hops, longest is 6 hops, max/min division is 2.0
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertEqual(columns[0][i], 2.0)

            # For all pairs, the distribution how many times they changed path
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/path/data/ecdf_time_step_num_path_changes.txt",
                "float,pos_float"
            )
            start_cumulative = 0.0
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], start_cumulative)
                else:
                    self.assertGreater(columns[1][i], start_cumulative)
                if i - 1 == range(len(columns[0])):
                    self.assertEqual(columns[1][i], 1.0)

                # There are only 5 time moments, none of which overlap, so this needs to be 5 times 1
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                elif i > 2000 - 6:
                    self.assertEqual(columns[0][i], 1.0)
                else:
                    self.assertEqual(columns[0][i], 0)

            # Analyze RTTs
            satgen.post_analysis.analyze_rtt(
                output_analysis_data_dir + "/" + name,
                output_generated_data_dir + "/" + name,
                default_time_step_ms,
                duration_s,
                ""
            )

            # Min. RTT
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_min_rtt_ns.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Only one pair with minimum RTT 25ish
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], 25229775.250687573, delta=100)

            # Max. RTT
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_rtt_ns.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Only one pair with max. RTT 48ish
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], 48165140.010532916, delta=100)

            # Max. - Min. RTT
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_minus_min_rtt_ns.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Only one pair with minimum RTT of 25ish, max. RTT is 48ish
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], 48165140.010532916 - 25229775.250687573, delta=100)

            # Max. / Min. RTT
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_rtt_to_min_rtt_slowdown.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Only one pair with minimum RTT of 25ish, max. RTT is 48ish
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], 48165140.010532916 / 25229775.250687573, delta=0.01)

            # Geodesic slowdown
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_rtt_to_geodesic_slowdown.txt",
                "float,pos_float"
            )
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Distance Manila to Dalian is 2,703 km according to Google Maps, RTT = 2*D / c
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], 48165140.010532916 / (2 * 2703000 / 0.299792), delta=0.01)

            # Analyze time step paths
            satgen.post_analysis.analyze_time_step_path(
                output_analysis_data_dir + "/" + name,
                output_generated_data_dir + "/" + name,
                all_time_step_ms,
                duration_s
            )

            # Missed path changes
            for time_step_ms in all_time_step_ms:
                columns = exputil.read_csv_direct_in_columns(
                    output_analysis_data_dir + "/" + name +
                    "/" + name + "/200s/path/data/"
                    + "ecdf_pairs_" + str(time_step_ms) + "ms_missed_path_changes.txt",
                    "float,pos_float"
                )
                for i in range(len(columns[0])):

                    # Cumulative y-axis check
                    if i == 0:
                        self.assertEqual(columns[1][i], 0)
                    else:
                        self.assertEqual(columns[1][i], 1.0)

                    # Only one should have missed for the 10s one
                    if i == 0:
                        self.assertEqual(columns[0][i], float("-inf"))
                    else:
                        if time_step_ms == 10000:
                            self.assertEqual(columns[0][i], 1)
                        else:
                            self.assertEqual(columns[0][i], 0)

            # Time between path changes
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/200s/path/data/"
                + "ecdf_overall_time_between_path_change.txt",
                "float,pos_float"
            )
            self.assertEqual(len(columns[0]), 5)  # Total 5 path changes, but only 4 of them are not from epoch
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    if i == 1:
                        self.assertEqual(columns[1][i], 0.25)
                    elif i == 2:
                        self.assertEqual(columns[1][i], 0.5)
                    elif i == 3:
                        self.assertEqual(columns[1][i], 0.75)
                    elif i == 4:
                        self.assertEqual(columns[1][i], 1.0)

                # Gap values
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    if i == 1:
                        self.assertEqual(columns[0][i], 2750000000)
                    elif i == 2:
                        self.assertEqual(columns[0][i], 9600000000)
                    elif i == 3:
                        self.assertEqual(columns[0][i], 46700000000)
                    elif i == 4:
                        self.assertEqual(columns[0][i], 51650000000)

            # Clean up
            local_shell.remove_force_recursive("temp_gen_data")
            local_shell.remove_force_recursive("temp_analysis_data")
