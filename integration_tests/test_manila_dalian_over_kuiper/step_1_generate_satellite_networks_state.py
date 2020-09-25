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

import sys
sys.path.append("../../satgenpy")
import satgen
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

# Full shell sizes
NUM_ORBS = 34
NUM_SATS_PER_ORB = 34

##
# TO SELECT THE NODES SET FROM THE FULL SHELL RUN
#
# utilization_filename = "a_b/runs/kuiper_630_isls_1173_to_1241_with_TcpNewReno_at_10_Mbps/logs_ns3/isl_utilization.csv"
# active_nodes = set()
# with open(utilization_filename) as f_in:
#     for line in f_in:
#         x = line.split(",")
#         if float(x[4]) != 0:
#             active_nodes.add(int(x[0]))
#             active_nodes.add(int(x[1]))
# print(active_nodes)
#
# active_links = set()
# with open(utilization_filename) as f_in:
#     for line in f_in:
#         x = line.split(",")
#         if float(x[4]) != 0:
#             active_links.add((int(x[0]), int(x[1])))
# sorted_active_links = sorted(list(active_links))
# for a in sorted_active_links:
#     if int(a[0]) <= int(a[1]):
#         print(a)
#     elif (int(a[1]), int(a[0])) not in sorted_active_links:
#         print((int(a[1]), int(a[0])))
#

local_shell = exputil.LocalShell()

# Clean slate start
local_shell.remove_force_recursive("temp/gen_data")
local_shell.make_full_dir("temp/gen_data")

# Both dynamic state algorithms should yield the same path and RTT
for dynamic_state_algorithm in [
    "algorithm_free_one_only_over_isls",
    "algorithm_free_gs_one_sat_many_only_over_isls"
]:

    # Specific outcomes
    output_generated_data_dir = "temp/gen_data"
    num_threads = 1
    time_step_ms = 100
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

    # Nodes
    #
    # They were chosen based on selecting only the satellites which
    # saw any utilization during a run over the full Kuiper constellation
    # (which takes too long to generate forwarding state for)
    #
    # Original ID   Test ID
    # 183           0
    # 184           1
    # 215           2
    # 216           3
    # 217           4
    # 218           5
    # 249           6
    # 250           7
    # 615           8
    # 616           9
    # 647           10
    # 648           11
    # 649           12
    # 650           13
    # 682           14
    # 683           15
    # 684           16

    limited_satellite_set = {
        183, 184,
        215, 216, 217, 218, 249, 250,
        615, 616,
        647, 648, 649, 650,
        682, 683, 684
    }
    limited_satellite_idx_map = {
        183: 0,
        184: 1,
        215: 2,
        216: 3,
        217: 4,
        218: 5,
        249: 6,
        250: 7,
        615: 8,
        616: 9,
        647: 10,
        648: 11,
        649: 12,
        650: 13,
        682: 14,
        683: 15,
        684: 16
    }

    # Create output directories
    if not os.path.isdir(output_generated_data_dir):
        os.makedirs(output_generated_data_dir)
    if not os.path.isdir(output_generated_data_dir + "/" + name):
        os.makedirs(output_generated_data_dir + "/" + name)

    # Ground stations
    print("Generating ground stations...")
    with open(output_generated_data_dir + "/" + name + "/ground_stations.basic.txt", "w+") as f_out:
        f_out.write("0,Manila,14.6042,120.9822,0\n")  # Originally no. 17 in top 100
        f_out.write("1,Dalian,38.913811,121.602322,0\n")  # Originally no. 85 in top 100
    satgen.extend_ground_stations(
        output_generated_data_dir + "/" + name + "/ground_stations.basic.txt",
        output_generated_data_dir + "/" + name + "/ground_stations.txt"
    )

    # TLEs (taken from Kuiper-610 first shell)
    print("Generating TLEs...")
    with open(output_generated_data_dir + "/" + name + "/tles.txt", "w+") as f_out:
        f_out.write("1 17\n")  # Pretend it's one orbit with 17 satellites
        f_out.write("Kuiper-630 0\n")  # 183
        f_out.write("1 00184U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    06\n")
        f_out.write("2 00184  51.9000  52.9412 0000001   0.0000 142.9412 14.80000000    00\n")
        f_out.write("Kuiper-630 1\n")  # 184
        f_out.write("1 00185U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    07\n")
        f_out.write("2 00185  51.9000  52.9412 0000001   0.0000 153.5294 14.80000000    07\n")
        f_out.write("Kuiper-630 2\n")  # 215
        f_out.write("1 00216U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    02\n")
        f_out.write("2 00216  51.9000  63.5294 0000001   0.0000 116.4706 14.80000000    04\n")
        f_out.write("Kuiper-630 3\n")  # 216
        f_out.write("1 00217U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    03\n")
        f_out.write("2 00217  51.9000  63.5294 0000001   0.0000 127.0588 14.80000000    01\n")
        f_out.write("Kuiper-630 4\n")  # 217
        f_out.write("1 00218U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    04\n")
        f_out.write("2 00218  51.9000  63.5294 0000001   0.0000 137.6471 14.80000000    00\n")
        f_out.write("Kuiper-630 5\n")  # 218
        f_out.write("1 00219U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05\n")
        f_out.write("2 00219  51.9000  63.5294 0000001   0.0000 148.2353 14.80000000    08\n")
        f_out.write("Kuiper-630 6\n")  # 249
        f_out.write("1 00250U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    00\n")
        f_out.write("2 00250  51.9000  74.1176 0000001   0.0000 121.7647 14.80000000    02\n")
        f_out.write("Kuiper-630 7\n")  # 250
        f_out.write("1 00251U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    01\n")
        f_out.write("2 00251  51.9000  74.1176 0000001   0.0000 132.3529 14.80000000    00\n")
        f_out.write("Kuiper-630 8\n")  # 615
        f_out.write("1 00616U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    06\n")
        f_out.write("2 00616  51.9000 190.5882 0000001   0.0000  31.7647 14.80000000    05\n")
        f_out.write("Kuiper-630 9\n")  # 616
        f_out.write("1 00617U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    07\n")
        f_out.write("2 00617  51.9000 190.5882 0000001   0.0000  42.3529 14.80000000    03\n")
        f_out.write("Kuiper-630 10\n")  # 647
        f_out.write("1 00648U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    01\n")
        f_out.write("2 00648  51.9000 201.1765 0000001   0.0000  15.8824 14.80000000    09\n")
        f_out.write("Kuiper-630 11\n")  # 648
        f_out.write("1 00649U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    02\n")
        f_out.write("2 00649  51.9000 201.1765 0000001   0.0000  26.4706 14.80000000    07\n")
        f_out.write("Kuiper-630 12\n")  # 649
        f_out.write("1 00650U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    04\n")
        f_out.write("2 00650  51.9000 201.1765 0000001   0.0000  37.0588 14.80000000    05\n")
        f_out.write("Kuiper-630 13\n")  # 650
        f_out.write("1 00651U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    05\n")
        f_out.write("2 00651  51.9000 201.1765 0000001   0.0000  47.6471 14.80000000    04\n")
        f_out.write("Kuiper-630 14\n")  # 682
        f_out.write("1 00683U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    00\n")
        f_out.write("2 00683  51.9000 211.7647 0000001   0.0000  21.1765 14.80000000    08\n")
        f_out.write("Kuiper-630 15\n")  # 683
        f_out.write("1 00684U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    01\n")
        f_out.write("2 00684  51.9000 211.7647 0000001   0.0000  31.7647 14.80000000    05\n")
        f_out.write("Kuiper-630 16\n")  # 684
        f_out.write("1 00685U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    02\n")
        f_out.write("2 00685  51.9000 211.7647 0000001   0.0000  42.3529 14.80000000    03\n")

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
            if isl[0] in limited_satellite_set and isl[1] in limited_satellite_set:
                f_out.write("%d %d\n" % (
                    limited_satellite_idx_map[isl[0]], limited_satellite_idx_map[isl[1]]
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
        17,  # 17 satellites
        len(ground_stations),
        gsl_interfaces_per_satellite,  # GSL interfaces per satellite
        1,  # (GSL) Interfaces per ground station
        gsl_satellite_max_agg_bandwidth,  # Aggregate max. bandwidth satellite (unit unspecified)
        1   # Aggregate max. bandwidth ground station (same unspecified unit)
    )

    # Forwarding state
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

    # TODO: Add parameter to specify where plotting files are
    # # Clean slate start
    # local_shell.remove_force_recursive("temp/analysis_data")
    # local_shell.make_full_dir("temp/analysis_data")
    # output_analysis_data_dir = "temp/analysis_data"
    # satgen.post_analysis.print_routes_and_rtt(
    #     output_analysis_data_dir + "/" + name,
    #     output_generated_data_dir + "/" + name,
    #     time_step_ms,
    #     duration_s,
    #     17,
    #     18
    # )
