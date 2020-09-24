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
import os
import shutil


# GENERATION CONSTANTS

BASE_NAME = "25x25"
NICE_NAME = "25x25-Legacy"

# 25x25

MAX_GSL_LENGTH_M = 1089686
MAX_ISL_LENGTH_M = 1000000000
NUM_ORBS = 25
NUM_SATS_PER_ORB = 25

# (Sao-Paolo - Moscow: 632 (7) to 651 (26))

################################################################


def calculate(duration_s, time_step_ms, dynamic_state_algorithm, num_threads):

    # Add base name to setting
    name = BASE_NAME + "_" + dynamic_state_algorithm

    # Create output directories
    if not os.path.isdir("gen_data"):
        os.makedirs("gen_data")
    if not os.path.isdir("gen_data/" + name):
        os.makedirs("gen_data/" + name)

    # Ground stations
    print("Generating ground stations...")
    shutil.copy("input_data/legacy/ground_stations_first_100.txt", "gen_data/" + name + "/ground_stations.txt")

    # TLEs
    print("Generating TLEs...")
    shutil.copy("input_data/legacy/starlink_tles_25x25.txt", "gen_data/" + name + "/tles.txt")

    # ISLs
    print("Generating ISLs...")
    satgen.generate_plus_grid_isls(
        "gen_data/" + name + "/isls.txt",
        NUM_ORBS,
        NUM_SATS_PER_ORB,
        isl_shift=1,
        idx_offset=0
    )

    # Description
    print("Generating description...")
    satgen.generate_description(
        "gen_data/" + name + "/description.txt",
        MAX_GSL_LENGTH_M,
        MAX_ISL_LENGTH_M
    )

    # GSL interfaces
    ground_stations = satgen.read_ground_stations_extended("gen_data/" + name + "/ground_stations.txt")
    if dynamic_state_algorithm == "algorithm_free_one_only_over_isls" \
            or dynamic_state_algorithm == "algorithm_free_one_only_gs_relays":
        gsl_interfaces_per_satellite = 1
    elif dynamic_state_algorithm == "algorithm_paired_many_only_over_isls":
        gsl_interfaces_per_satellite = len(ground_stations)
    else:
        raise ValueError("Unknown dynamic state algorithm")

    print("Generating GSL interfaces info..")
    satgen.generate_simple_gsl_interfaces_info(
        "gen_data/" + name + "/gsl_interfaces_info.txt",
        NUM_ORBS * NUM_SATS_PER_ORB,
        len(ground_stations),
        gsl_interfaces_per_satellite,  # GSL interfaces per satellite
        1,  # (GSL) Interfaces per ground station
        1,  # Aggregate max. bandwidth satellite (unit unspecified)
        1   # Aggregate max. bandwidth ground station (same unspecified unit)
    )

    # Forwarding state
    print("Generating forwarding state...")
    satgen.help_dynamic_state(
        "gen_data",
        num_threads,  # Number of threads
        name,
        time_step_ms,
        duration_s,
        MAX_GSL_LENGTH_M,
        MAX_ISL_LENGTH_M,
        dynamic_state_algorithm,
        True
    )


def main():
    args = sys.argv[1:]
    if len(args) != 4:
        print("Must supply exactly four arguments")
        print("Usage: python main_25x25.py [duration (s)] [time step (ms)] "
              "[algorithm_{free_one_only_over_isls, free_one_only_gs_relays, paired_many_only_over_isls}] "
              "[num threads]")
        exit(1)
    else:
        calculate(
            int(args[0]),
            int(args[1]),
            args[2],
            int(args[3]),
        )


if __name__ == "__main__":
    main()
