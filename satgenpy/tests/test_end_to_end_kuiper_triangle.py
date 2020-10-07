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

# GENERATION CONSTANTS

BASE_NAME = "kuiper_630"
NICE_NAME = "Kuiper-630"

# KUIPER 630

ECCENTRICITY = 0.0000001  # Circular orbits are zero, but pyephem does not permit 0, so lowest possible value
ARG_OF_PERIGEE_DEGREE = 0.0
PHASE_DIFF = True

################################################################
# The below constants are taken from Kuiper's FCC filing as below:
# [1]: https://www.itu.int/ITU-R/space/asreceived/Publication/DisplayPublication/8716
################################################################

MEAN_MOTION_REV_PER_DAY = 14.80  # Altitude ~630 km
ALTITUDE_M = 630000  # Altitude ~630 km

# Considering an elevation angle of 30 degrees; possible values [1]: 20(min)/30/35/45
SATELLITE_CONE_RADIUS_M = ALTITUDE_M / math.tan(math.radians(30.0))

MAX_GSL_LENGTH_M = math.sqrt(math.pow(SATELLITE_CONE_RADIUS_M, 2) + math.pow(ALTITUDE_M, 2))

# ISLs are not allowed to dip below 80 km altitude in order to avoid weather conditions
MAX_ISL_LENGTH_M = 2 * math.sqrt(math.pow(EARTH_RADIUS + ALTITUDE_M, 2) - math.pow(EARTH_RADIUS + 80000, 2))

NUM_ORBS = 34
NUM_SATS_PER_ORB = 34
INCLINATION_DEGREE = 51.9


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
            all_time_step_ms = [50, 100, 1000, 10000, 20000]
            duration_s = 200

            # Add base name to setting
            name = "triangle_reduced_kuiper_630_" + dynamic_state_algorithm

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
                f_out.write("2,Sankt-Peterburg-(Saint-Petersburg),59.929858,30.326228,0\n")  # Originally no. 73
            satgen.extend_ground_stations(
                output_generated_data_dir + "/" + name + "/ground_stations.basic.txt",
                output_generated_data_dir + "/" + name + "/ground_stations.txt"
            )

            # Path trace we base this test on:

            # (1) 1173 -> 1241
            # 0,1173-184-183-217-1241
            # 18000000000,1173-218-217-1241
            # 27600000000,1173-648-649-650-616-1241
            # 74300000000,1173-218-217-216-250-1241
            # 125900000000,1173-647-648-649-650-616-1241
            # 128700000000,1173-647-648-649-615-1241

            # (2) 1229 -> 1241
            # 0,1229-144-178-212-246-280-281-282-283-1241
            # 3300000000,1229-177-178-212-246-280-281-282-283-1241
            # 10100000000,1229-177-178-212-246-247-248-249-1241
            # 128700000000,1229-177-211-245-246-247-248-249-1241
            # 139500000000,1229-144-178-212-246-247-248-249-1241
            # 155400000000,Unreachable
            # 165200000000,1229-143-177-211-245-279-280-281-282-1241
            # 178800000000,1229-176-177-211-245-279-280-281-282-1241

            # (3) 1229 -> 1173
            # 0,1229-144-178-179-180-181-182-183-184-1173
            # 3300000000,1229-177-178-179-180-181-182-183-184-1173
            # 139500000000,1229-144-178-179-180-181-182-183-184-1173
            # 150100000000,1229-144-178-179-180-181-182-183-1173
            # 155400000000,Unreachable
            # 165200000000,1229-143-177-178-179-180-181-182-183-1173
            # 178800000000,1229-176-177-178-179-180-181-182-183-1173

            # Select all satellite IDs
            subset_of_satellites = set()
            for path_filename in [
                "tests/data_to_match/kuiper_630/networkx_path_1173_to_1241.txt",
                "tests/data_to_match/kuiper_630/networkx_path_1229_to_1173.txt",
                "tests/data_to_match/kuiper_630/networkx_path_1229_to_1241.txt",
            ]:
                columns = exputil.read_csv_direct_in_columns(path_filename, "pos_int,string")
                for path in columns[1]:
                    if path != "Unreachable":
                        for sat_id in list(map(lambda x: int(x), path.split("-")[1:-1])):
                            subset_of_satellites.add(sat_id)
            list_of_satellites = sorted(list(subset_of_satellites))
            original_sat_id_to_new_sat_id = {}
            for i in range(len(list_of_satellites)):
                original_sat_id_to_new_sat_id[list_of_satellites[i]] = i

            # Generate normal TLEs and then only filter out the limited satellite list
            print("Generating TLEs...")
            satgen.generate_tles_from_scratch_manual(
                output_generated_data_dir + "/" + name + "/tles_complete.txt",
                NICE_NAME,
                NUM_ORBS,
                NUM_SATS_PER_ORB,
                PHASE_DIFF,
                INCLINATION_DEGREE,
                ECCENTRICITY,
                ARG_OF_PERIGEE_DEGREE,
                MEAN_MOTION_REV_PER_DAY
            )
            with open(output_generated_data_dir + "/" + name + "/tles_complete.txt", "r") as f_in:
                with open(output_generated_data_dir + "/" + name + "/tles.txt", "w+") as f_out:
                    f_out.write("1 %d\n" % len(list_of_satellites))  # Pretend its one orbit with N satellites simply
                    i = 0
                    for line in f_in:
                        line = line.strip()
                        if int(math.floor((i - 1) / 3.0)) in list_of_satellites:
                            if (i - 1) % 3 == 0:
                                f_out.write("%s %d\n" % (
                                    line.split(" ")[0],
                                    original_sat_id_to_new_sat_id[int(line.split(" ")[1])]
                                ))
                            else:
                                f_out.write("%s\n" % line)
                        i += 1

            # ISLs
            print("Generating ISLs...")
            complete_list_isls = satgen.generate_plus_grid_isls(
                output_generated_data_dir + "/" + name + "/isls_complete.temp.txt",
                NUM_ORBS,
                NUM_SATS_PER_ORB,
                isl_shift=0,
                idx_offset=0
            )
            with open(output_generated_data_dir + "/" + name + "/isls.txt", "w+") as f_out:
                for isl in complete_list_isls:
                    if isl[0] in list_of_satellites and isl[1] in list_of_satellites:
                        f_out.write("%d %d\n" % (
                            original_sat_id_to_new_sat_id[isl[0]], original_sat_id_to_new_sat_id[isl[1]]
                        ))

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
                len(list_of_satellites),  # N satellites
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

            # Check the path and RTT for each pair
            new_gs_id_to_old_node_id = {
                0: 1173,
                1: 1241,
                2: 1229
            }
            old_node_id_to_new_node_id = {
                1173: len(list_of_satellites) + 0,
                1241: len(list_of_satellites) + 1,
                1229: len(list_of_satellites) + 2,
            }
            min_rtts = []
            max_rtts = []
            for (src, dst) in [
                (0, 1),
                (0, 2),
                (1, 0),
                (1, 2),
                (2, 0),
                (2, 1)
            ]:

                # Find node identifiers
                src_node_id = len(list_of_satellites) + src
                dst_node_id = len(list_of_satellites) + dst
                old_src_node_id = new_gs_id_to_old_node_id[src]
                old_dst_node_id = new_gs_id_to_old_node_id[dst]

                # Print the routes
                satgen.post_analysis.print_routes_and_rtt(
                    output_analysis_data_dir + "/" + name,
                    output_generated_data_dir + "/" + name,
                    default_time_step_ms,
                    duration_s,
                    src_node_id,
                    dst_node_id,
                    ""
                )

                # Now, we just want to see that the output path matches
                with open(
                        output_analysis_data_dir + "/" + name + "/data/networkx_path_%d_to_%d.txt"
                        % (src_node_id, dst_node_id),
                        "r"
                ) as f_in1:
                    with open(
                            "tests/data_to_match/kuiper_630/networkx_path_%d_to_%d.txt"
                            % (old_src_node_id, old_dst_node_id),
                            "r"
                    ) as f_in2:
                        lines1 = []
                        for line in f_in1:
                            lines1.append(line.strip())
                        lines2 = []
                        for line in f_in2:
                            lines2.append(line.strip())
                        self.assertEqual(len(lines1), len(lines2))
                        for i in range(len(lines1)):
                            spl1 = lines1[i].split(",")
                            spl2 = lines2[i].split(",")

                            # Time must be equal
                            self.assertEqual(spl1[0], spl2[0])

                            # Path must be equal
                            if spl1[1] == "Unreachable" or spl2[1] == "Unreachable":
                                self.assertEqual(spl1[1], spl2[1])
                            else:
                                node_list1 = list(map(lambda x: int(x), spl1[1].split("-")))
                                node_list2 = list(map(lambda x: int(x), spl2[1].split("-")))
                                new_node_list2 = []
                                for j in range(len(node_list2)):
                                    if j == 0 or j == len(node_list2) - 1:
                                        new_node_list2.append(old_node_id_to_new_node_id[node_list2[j]])
                                    else:
                                        new_node_list2.append(original_sat_id_to_new_sat_id[node_list2[j]])
                                self.assertEqual(
                                    node_list1,
                                    new_node_list2
                                )

                # ... and the RTT
                lowest_rtt_ns = 100000000000
                highest_rtt_ns = 0
                with open(
                        output_analysis_data_dir + "/" + name + "/data/networkx_rtt_%d_to_%d.txt"
                        % (src_node_id, dst_node_id),
                        "r"
                ) as f_in1:
                    with open(
                            "tests/data_to_match/kuiper_630/networkx_rtt_%d_to_%d.txt"
                            % (old_src_node_id, old_dst_node_id),
                            "r"
                    ) as f_in2:
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
                            if a_rtt != 0:
                                lowest_rtt_ns = min(a_rtt, lowest_rtt_ns)
                                highest_rtt_ns = max(a_rtt, highest_rtt_ns)
                            self.assertEqual(a_time, b_time)
                            self.assertAlmostEqual(a_rtt, b_rtt, places=5)

                # Save RTTs
                if src < dst:
                    min_rtts.append(lowest_rtt_ns)
                    max_rtts.append(highest_rtt_ns)

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
            self.assertEqual(4, len(columns[0]))
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # There are three pairs, with 5, 6 and 7 path changes
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                elif i == 1:
                    self.assertEqual(columns[0][i], 5.0)
                elif i == 2:
                    self.assertEqual(columns[0][i], 6.0)
                else:
                    self.assertEqual(columns[0][i], 7.0)

            # Max minus min hop count per pair
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/path/data/ecdf_pairs_max_minus_min_hop_count.txt",
                "float,pos_float"
            )
            self.assertEqual(4, len(columns[0]))
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # One with 3 vs. 6, and two with 8 vs. 9
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                elif i == 1:
                    self.assertEqual(columns[0][i], 1)
                elif i == 2:
                    self.assertEqual(columns[0][i], 1)
                else:
                    self.assertEqual(columns[0][i], 3)

            # Max divided by min hop count per pair
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/path/data/ecdf_pairs_max_hop_count_to_min_hop_count.txt",
                "float,pos_float"
            )
            self.assertEqual(4, len(columns[0]))
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # One with 3 vs. 6, and two with 8 vs. 9
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                elif i == 1:
                    self.assertAlmostEqual(columns[0][i], 9.0/8.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[0][i], 9.0/8.0, delta=0.0001)
                else:
                    self.assertEqual(columns[0][i], 2.0)

            # These are the path changes

            # 18000000000,1173-218-217-1241

            # 27600000000,1173-648-649-650-616-1241

            # 3300000000,1229-177-178-179-180-181-182-183-184-1173
            # 3300000000,1229-177-178-212-246-280-281-282-283-1241

            # 74300000000,1173-218-217-216-250-1241

            # 10100000000,1229-177-178-212-246-247-248-249-1241

            # 125900000000,1173-647-648-649-650-616-1241

            # 128700000000,1229-177-211-245-246-247-248-249-1241
            # 128700000000,1173-647-648-649-615-1241

            # 139500000000,1229-144-178-179-180-181-182-183-184-1173
            # 139500000000,1229-144-178-212-246-247-248-249-1241

            # 150100000000,1229-144-178-179-180-181-182-183-1173

            # 155400000000,Unreachable
            # 155400000000,Unreachable

            # 165200000000,1229-143-177-211-245-279-280-281-282-1241
            # 165200000000,1229-143-177-178-179-180-181-182-183-1173

            # 178800000000,1229-176-177-211-245-279-280-281-282-1241
            # 178800000000,1229-176-177-178-179-180-181-182-183-1173

            # For all pairs, the distribution how many times they changed path in a time step
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

                # There are 12 time steps, of which 6 have 2 changes, and 6 have 1 change
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                elif i > 2000 - 7:
                    self.assertEqual(columns[0][i], 2.0)
                elif i > 2000 - 13:
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
            self.assertEqual(4, len(columns[0]))
            sorted_min_rtts = sorted(min_rtts)
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # RTT
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], sorted_min_rtts[i - 1], delta=100)

            # Max. RTT
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_rtt_ns.txt",
                "float,pos_float"
            )
            self.assertEqual(4, len(columns[0]))
            sorted_max_rtts = sorted(max_rtts)
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # RTT
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], sorted_max_rtts[i - 1], delta=100)

            # Max. - Min. RTT
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_minus_min_rtt_ns.txt",
                "float,pos_float"
            )
            self.assertEqual(4, len(columns[0]))
            sorted_max_minus_min_rtts = sorted(list(map(lambda x: max_rtts[x] - min_rtts[x], list(range(3)))))
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # RTT
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], sorted_max_minus_min_rtts[i - 1], delta=100)

            # Max. / Min. RTT
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_rtt_to_min_rtt_slowdown.txt",
                "float,pos_float"
            )
            self.assertEqual(4, len(columns[0]))
            sorted_max_divided_min_rtts = sorted(list(map(lambda x: max_rtts[x] / min_rtts[x], list(range(3)))))
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # RTT
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], sorted_max_divided_min_rtts[i - 1], delta=0.01)

            # Geodesic slowdown
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/100ms_for_200s/rtt/data/ecdf_pairs_max_rtt_to_geodesic_slowdown.txt",
                "float,pos_float"
            )
            # From Google search
            # Distance Manila to Dalian is 2,703 km according to Google Maps
            # Distance St. Petersburg to Manila  is 8,635 km according to Google Maps
            # Distance St. Petersburg to Dalian is 6,406 km according to Google Maps
            self.assertEqual(4, len(columns[0]))
            geodesic_expected_distance = [
                2703,
                8635,
                6406
            ]
            sorted_max_divided_geodesic_rtts = sorted(list(map(
                    lambda x: max_rtts[x] / (2 * geodesic_expected_distance[x] * 1000.0 / 0.299792),
                    list(range(3))
            )))
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                elif i == 1:
                    self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                elif i == 2:
                    self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                else:
                    self.assertEqual(columns[1][i], 1.0)

                # Geodesic RTT = 2*D / c
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    self.assertAlmostEqual(columns[0][i], sorted_max_divided_geodesic_rtts[i - 1], delta=0.01)

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
                    elif i == 1:
                        self.assertAlmostEqual(columns[1][i], 1.0/3.0, delta=0.0001)
                    elif i == 2:
                        self.assertAlmostEqual(columns[1][i], 2.0/3.0, delta=0.0001)
                    else:
                        self.assertEqual(columns[1][i], 1.0)

                    # Only two should have missed one for the 10s one
                    # 1, 2 and 3 respectively at 20s
                    if i == 0:
                        self.assertEqual(columns[0][i], float("-inf"))
                    else:
                        if time_step_ms == 10000:
                            if i == 1:
                                self.assertEqual(columns[0][i], 0)
                            if i == 2:
                                self.assertEqual(columns[0][i], 1)
                            if i == 3:
                                self.assertEqual(columns[0][i], 1)
                        elif time_step_ms == 20000:
                            if i == 1:
                                self.assertEqual(columns[0][i], 1)
                            if i == 2:
                                self.assertEqual(columns[0][i], 2)
                            if i == 3:
                                self.assertEqual(columns[0][i], 3)
                        else:
                            self.assertEqual(columns[0][i], 0)

            # Time between path changes
            columns = exputil.read_csv_direct_in_columns(
                output_analysis_data_dir + "/" + name +
                "/" + name + "/200s/path/data/"
                + "ecdf_overall_time_between_path_change.txt",
                "float,pos_float"
            )
            # Total 18 path changes, but only 15 of them are not from epoch (plus one for (0, -inf))
            self.assertEqual(len(columns[0]), 16)
            for i in range(len(columns[0])):

                # Cumulative y-axis check
                if i == 0:
                    self.assertEqual(columns[1][i], 0)
                else:
                    self.assertAlmostEqual(columns[1][i], i / float(len(columns[0]) - 1), delta=0.00001)

                # Gap values
                if i == 0:
                    self.assertEqual(columns[0][i], float("-inf"))
                else:
                    if i == 1:
                        self.assertEqual(columns[0][i], 2750000000)
                    elif i == 2:
                        self.assertEqual(columns[0][i], 5350000000)
                    elif i == 3:
                        self.assertEqual(columns[0][i], 6800000000)
                    elif i == 4:
                        self.assertEqual(columns[0][i], 9600000000)
                    elif i == 5:
                        self.assertEqual(columns[0][i], 9750000000)
                    elif i == 6:
                        self.assertEqual(columns[0][i], 9750000000)
                    elif i == 7:
                        self.assertEqual(columns[0][i], 10550000000)
                    elif i == 8:
                        self.assertEqual(columns[0][i], 10800000000)
                    elif i == 9:
                        self.assertEqual(columns[0][i], 13600000000)
                    elif i == 10:
                        self.assertEqual(columns[0][i], 13600000000)
                    elif i == 11:
                        self.assertEqual(columns[0][i], 15900000000)
                    elif i == 12:
                        self.assertEqual(columns[0][i], 46700000000)
                    elif i == 13:
                        self.assertEqual(columns[0][i], 51650000000)
                    elif i == 14:
                        self.assertEqual(columns[0][i], 118650000000)
                    elif i == 15:
                        self.assertEqual(columns[0][i], 136250000000)

            # Clean up
            local_shell.remove_force_recursive("temp_gen_data")
            local_shell.remove_force_recursive("temp_analysis_data")
