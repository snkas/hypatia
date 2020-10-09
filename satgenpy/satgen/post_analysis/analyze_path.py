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

from .graph_tools import *
from satgen.ground_stations import *
from satgen.tles import *
import exputil
import numpy as np
from .print_routes_and_rtt import print_routes_and_rtt
from statsmodels.distributions.empirical_distribution import ECDF


def analyze_path(
        output_data_dir, satellite_network_dir, dynamic_state_update_interval_ms,
        simulation_end_time_s, satgenpy_dir_with_ending_slash
):

    # Variables (load in for each thread such that they don't interfere)
    satellite_network_dynamic_state_dir = "%s/dynamic_state_%dms_for_%ds" % (
        satellite_network_dir, dynamic_state_update_interval_ms, simulation_end_time_s
    )
    ground_stations = read_ground_stations_extended(satellite_network_dir + "/ground_stations.txt")
    tles = read_tles(satellite_network_dir + "/tles.txt")
    satellites = tles["satellites"]

    # Local shell
    local_shell = exputil.LocalShell()
    core_network_folder_name = satellite_network_dir.split("/")[-1]
    base_output_dir = "%s/%s/%dms_for_%ds/path" % (
        output_data_dir, core_network_folder_name, dynamic_state_update_interval_ms, simulation_end_time_s
    )
    pdf_dir = base_output_dir + "/pdf"
    data_dir = base_output_dir + "/data"
    local_shell.remove_force_recursive(pdf_dir)
    local_shell.remove_force_recursive(data_dir)
    local_shell.make_full_dir(pdf_dir)
    local_shell.make_full_dir(data_dir)

    # Derivatives
    simulation_end_time_ns = simulation_end_time_s * 1000 * 1000 * 1000
    dynamic_state_update_interval_ns = dynamic_state_update_interval_ms * 1000 * 1000

    # Analysis
    path_list_per_pair = []
    for i in range(len(ground_stations)):
        temp_list = []
        for j in range(len(ground_stations)):
            temp_list.append([])
        path_list_per_pair.append(temp_list)

    # Time step analysis
    time_step_num_path_changes = []
    time_step_num_fstate_updates = []

    # For each time moment
    fstate = {}
    num_iterations = simulation_end_time_ns / dynamic_state_update_interval_ns
    it = 1
    for t in range(0, simulation_end_time_ns, dynamic_state_update_interval_ns):
        num_path_changes = 0
        num_fstate_updates = 0

        # Read in forwarding state
        with open(satellite_network_dynamic_state_dir + "/fstate_" + str(t) + ".txt", "r") as f_in:
            for line in f_in:
                spl = line.split(",")
                current = int(spl[0])
                destination = int(spl[1])
                next_hop = int(spl[2])
                fstate[(current, destination)] = next_hop
                num_fstate_updates += 1

            # Go over each pair of ground stations and calculate the length
            for src in range(len(ground_stations)):
                for dst in range(src + 1, len(ground_stations)):
                    src_node_id = len(satellites) + src
                    dst_node_id = len(satellites) + dst
                    path = get_path(src_node_id, dst_node_id, fstate)
                    if path is None:
                        if len(path_list_per_pair[src][dst]) == 0 or path_list_per_pair[src][dst][-1] != []:
                            path_list_per_pair[src][dst].append([])
                            num_path_changes += 1
                    else:
                        if len(path_list_per_pair[src][dst]) == 0 or path != path_list_per_pair[src][dst][-1]:
                            path_list_per_pair[src][dst].append(path)
                            num_path_changes += 1

        # First iteration has an update for all, which is not interesting
        # to show in the ECDF and is not really a "change" / "update"
        if it != 1:
            time_step_num_path_changes.append(num_path_changes)
            time_step_num_fstate_updates.append(num_fstate_updates)

        # Show progress a bit
        print("%d / %d" % (it, num_iterations))
        it += 1
    print("")

    # Calculate hop count list
    hop_count_list_per_pair = []
    for src in range(len(ground_stations)):
        temp_list = []
        for dst in range(len(ground_stations)):  # The one until src are empty, but those are ignored later
            r = []
            for x in path_list_per_pair[src][dst]:
                if len(x) != 0:
                    if len(x) < 2:
                        raise ValueError("Path must have 0 or at least 2 nodes")
                    r.append(len(x) - 1)  # Number of nodes - 1 is the hop count
            temp_list.append(r)
        hop_count_list_per_pair.append(temp_list)

    #################################################

    # ECDF stuff, which is quick, so we do that first

    # Find all the lists
    list_max_minus_min_hop_count = []
    list_max_hop_count_to_min_hop_count = []
    list_num_path_changes = []
    for src in range(len(ground_stations)):
        for dst in range(src + 1, len(ground_stations)):
            min_hop_count = np.min(hop_count_list_per_pair[src][dst])
            max_hop_count = np.max(hop_count_list_per_pair[src][dst])
            list_max_hop_count_to_min_hop_count.append(float(max_hop_count) / float(min_hop_count))
            list_max_minus_min_hop_count.append(max_hop_count - min_hop_count)
            list_num_path_changes.append(len(path_list_per_pair[src][dst]) - 1)  # First path is not a change, so - 1

    # Write and plot ECDFs
    for element in [
        ("ecdf_pairs_max_minus_min_hop_count", ECDF(list_max_minus_min_hop_count)),
        ("ecdf_pairs_max_hop_count_to_min_hop_count", ECDF(list_max_hop_count_to_min_hop_count)),
        ("ecdf_pairs_num_path_changes", ECDF(list_num_path_changes)),
        ("ecdf_time_step_num_path_changes", ECDF(time_step_num_path_changes)),
        ("ecdf_time_step_num_fstate_updates", ECDF(time_step_num_fstate_updates)),
    ]:
        name = element[0]
        ecdf = element[1]
        with open(data_dir + "/" + name + ".txt", "w+") as f_out:
            for i in range(len(ecdf.x)):
                f_out.write(str(ecdf.x[i]) + "," + str(ecdf.y[i]) + "\n")

    #################################################

    # Largest hop count delta
    with open(data_dir + "/top_10_largest_hop_count_delta.txt", "w+") as f_out:
        largest_hop_count_delta_list = []
        for src in range(len(ground_stations)):
            for dst in range(src + 1, len(ground_stations)):
                min_hop_count = np.min(hop_count_list_per_pair[src][dst])
                max_hop_count = np.max(hop_count_list_per_pair[src][dst])
                largest_hop_count_delta_list.append((max_hop_count - min_hop_count, min_hop_count, max_hop_count,
                                                     src, dst))
        largest_hop_count_delta_list = sorted(largest_hop_count_delta_list, reverse=True)
        f_out.write("LARGEST HOP-COUNT DELTA TOP-10 WITHOUT DUPLICATE NODES (EXCL. UNREACHABLE)\n")
        f_out.write("------------------------------------------------------------------\n")
        f_out.write("#      Pair              Delta         Min. hop count    Max. hop count\n")
        already_plotted_nodes = set()
        num_plotted = 0
        for i in range(len(largest_hop_count_delta_list)):
            if largest_hop_count_delta_list[i][3] not in already_plotted_nodes \
                    and largest_hop_count_delta_list[i][4] not in already_plotted_nodes:
                f_out.write("%-3d    %-4d -> %4d       %8d     %-8d          %-8d\n" % (
                    i + 1,
                    len(satellites) + largest_hop_count_delta_list[i][3],
                    len(satellites) + largest_hop_count_delta_list[i][4],
                    largest_hop_count_delta_list[i][0],
                    largest_hop_count_delta_list[i][1],
                    largest_hop_count_delta_list[i][2],
                ))
                print_routes_and_rtt(base_output_dir, satellite_network_dir, dynamic_state_update_interval_ms,
                                     simulation_end_time_s, len(satellites) + largest_hop_count_delta_list[i][3],
                                     len(satellites) + largest_hop_count_delta_list[i][4],
                                     satgenpy_dir_with_ending_slash)
                already_plotted_nodes.add(largest_hop_count_delta_list[i][3])
                already_plotted_nodes.add(largest_hop_count_delta_list[i][4])
                num_plotted += 1
                if num_plotted >= 10:
                    break
        f_out.write("---------------------------------------------------------------\n")
        f_out.write("\n")

    # Number of path changes
    with open(data_dir + "/top_10_most_path_changes.txt", "w+") as f_out:
        most_path_changes_list = []
        for src in range(len(ground_stations)):
            for dst in range(src + 1, len(ground_stations)):
                most_path_changes_list.append((len(path_list_per_pair[src][dst]) - 1, src, dst))
        most_path_changes_list = sorted(most_path_changes_list, reverse=True)
        f_out.write("MOST PATH CHANGES TOP-10 WITHOUT DUPLICATE NODES\n")
        f_out.write("-------------------------------------\n")
        f_out.write("#      Pair           Number of path changes\n")
        already_plotted_nodes = set()
        num_plotted = 0
        for i in range(len(most_path_changes_list)):
            if most_path_changes_list[i][1] not in already_plotted_nodes \
                    and most_path_changes_list[i][2] not in already_plotted_nodes:
                f_out.write("%-3d    %-4d -> %4d   %d\n" % (
                    i + 1,
                    len(satellites) + most_path_changes_list[i][1],
                    len(satellites) + most_path_changes_list[i][2],
                    most_path_changes_list[i][0]
                ))
                print_routes_and_rtt(base_output_dir, satellite_network_dir,
                                     dynamic_state_update_interval_ms, simulation_end_time_s,
                                     len(satellites) + most_path_changes_list[i][1],
                                     len(satellites) + most_path_changes_list[i][2],
                                     satgenpy_dir_with_ending_slash)
                already_plotted_nodes.add(most_path_changes_list[i][1])
                already_plotted_nodes.add(most_path_changes_list[i][2])
                num_plotted += 1
                if num_plotted >= 10:
                    break
        f_out.write("---------------------------------------\n")
        f_out.write("\n")

    print("Done")
