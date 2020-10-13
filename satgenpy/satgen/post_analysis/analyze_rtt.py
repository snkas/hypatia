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
from satgen.distance_tools import *
from satgen.isls import *
from satgen.ground_stations import *
from satgen.tles import *
import exputil
import numpy as np
from .print_routes_and_rtt import print_routes_and_rtt
from statsmodels.distributions.empirical_distribution import ECDF


SPEED_OF_LIGHT_M_PER_S = 299792458.0

GEODESIC_ECDF_PLOT_CUTOFF_KM = 500


def analyze_rtt(
        output_data_dir, satellite_network_dir, dynamic_state_update_interval_ms,
        simulation_end_time_s, satgenpy_dir_with_ending_slash
):

    # Dynamic state directory
    satellite_network_dynamic_state_dir = "%s/dynamic_state_%dms_for_%ds" % (
        satellite_network_dir, dynamic_state_update_interval_ms, simulation_end_time_s
    )

    # Local shell
    local_shell = exputil.LocalShell()
    core_network_folder_name = satellite_network_dir.split("/")[-1]
    base_output_dir = "%s/%s/%dms_for_%ds/rtt" % (
        output_data_dir, core_network_folder_name, dynamic_state_update_interval_ms, simulation_end_time_s
    )
    pdf_dir = base_output_dir + "/pdf"
    data_dir = base_output_dir + "/data"
    local_shell.remove_force_recursive(pdf_dir)
    local_shell.remove_force_recursive(data_dir)
    local_shell.make_full_dir(pdf_dir)
    local_shell.make_full_dir(data_dir)

    # Variables (load in for each thread such that they don't interfere)
    ground_stations = read_ground_stations_extended(satellite_network_dir + "/ground_stations.txt")
    tles = read_tles(satellite_network_dir + "/tles.txt")
    satellites = tles["satellites"]
    list_isls = read_isls(satellite_network_dir + "/isls.txt", len(satellites))
    epoch = tles["epoch"]
    description = exputil.PropertiesConfig(satellite_network_dir + "/description.txt")

    # Derivatives
    simulation_end_time_ns = simulation_end_time_s * 1000 * 1000 * 1000
    dynamic_state_update_interval_ns = dynamic_state_update_interval_ms * 1000 * 1000
    max_gsl_length_m = exputil.parse_positive_float(description.get_property_or_fail("max_gsl_length_m"))
    max_isl_length_m = exputil.parse_positive_float(description.get_property_or_fail("max_isl_length_m"))

    # Analysis
    rtt_list_per_pair = []
    for i in range(len(ground_stations)):
        temp_list = []
        for j in range(len(ground_stations)):
            temp_list.append([])
        rtt_list_per_pair.append(temp_list)
    unreachable_per_pair = np.zeros((len(ground_stations), len(ground_stations)))

    # For each time moment
    fstate = {}
    num_iterations = simulation_end_time_ns / dynamic_state_update_interval_ns
    it = 1
    for t in range(0, simulation_end_time_ns, dynamic_state_update_interval_ns):

        # Read in forwarding state
        with open(satellite_network_dynamic_state_dir + "/fstate_" + str(t) + ".txt", "r") as f_in:
            for line in f_in:
                spl = line.split(",")
                current = int(spl[0])
                destination = int(spl[1])
                next_hop = int(spl[2])
                fstate[(current, destination)] = next_hop

            # Given we are going to graph often, we can pre-compute the edge lengths
            graph_with_distance = construct_graph_with_distances(epoch, t, satellites, ground_stations,
                                                                 list_isls, max_gsl_length_m, max_isl_length_m)

            # Go over each pair of ground stations and calculate the length
            for src in range(len(ground_stations)):
                for dst in range(src + 1, len(ground_stations)):
                    src_node_id = len(satellites) + src
                    dst_node_id = len(satellites) + dst
                    path = get_path(src_node_id, dst_node_id, fstate)
                    if path is None:
                        unreachable_per_pair[(src, dst)] += 1
                    else:
                        length_path_m = compute_path_length_with_graph(path, graph_with_distance)
                        rtt_list_per_pair[src][dst].append((2 * length_path_m) * 1000000000.0 / SPEED_OF_LIGHT_M_PER_S)

        # Show progress a bit
        print("%d / %d" % (it, num_iterations))
        it += 1
    print("")

    #################################################

    # ECDF stuff, which is quick, so we do that first

    # Find all the lists
    list_min_rtt_ns = []
    list_max_rtt_ns = []
    list_max_minus_min_rtt_ns = []
    list_max_rtt_to_min_rtt_slowdown = []
    list_max_rtt_to_geodesic_slowdown = []
    for src in range(len(ground_stations)):
        for dst in range(src + 1, len(ground_stations)):
            min_rtt_ns = np.min(rtt_list_per_pair[src][dst])
            max_rtt_ns = np.max(rtt_list_per_pair[src][dst])
            max_rtt_slowdown = float(max_rtt_ns) / float(min_rtt_ns)
            list_min_rtt_ns.append(min_rtt_ns)
            list_max_rtt_ns.append(max_rtt_ns)
            list_max_minus_min_rtt_ns.append(max_rtt_ns - min_rtt_ns)
            list_max_rtt_to_min_rtt_slowdown.append(max_rtt_slowdown)
            geodesic_distance_m = geodesic_distance_m_between_ground_stations(
                ground_stations[src],
                ground_stations[dst]
            )
            # If the geodesic is under 500km, we do not consider it,
            # as one would use terrestrial networks vs. expending the effort to go up and down
            # Especially if populated cities are very close to each other, would this give a large geodesic slow-down
            if geodesic_distance_m >= GEODESIC_ECDF_PLOT_CUTOFF_KM * 1000:
                geodesic_rtt_ns = geodesic_distance_m * 2 * 1000000000.0 / SPEED_OF_LIGHT_M_PER_S
                list_max_rtt_to_geodesic_slowdown.append(float(max_rtt_ns) / float(geodesic_rtt_ns))

    # Write and plot ECDFs
    for element in [
        ("ecdf_pairs_min_rtt_ns", ECDF(list_min_rtt_ns)),
        ("ecdf_pairs_max_rtt_ns", ECDF(list_max_rtt_ns)),
        ("ecdf_pairs_max_minus_min_rtt_ns", ECDF(list_max_minus_min_rtt_ns)),
        ("ecdf_pairs_max_rtt_to_min_rtt_slowdown", ECDF(list_max_rtt_to_min_rtt_slowdown)),
        ("ecdf_pairs_max_rtt_to_geodesic_slowdown", ECDF(list_max_rtt_to_geodesic_slowdown)),
    ]:
        name = element[0]
        ecdf = element[1]
        with open(data_dir + "/" + name + ".txt", "w+") as f_out:
            for i in range(len(ecdf.x)):
                f_out.write(str(ecdf.x[i]) + "," + str(ecdf.y[i]) + "\n")

    #################################################

    # Largest RTT delta
    with open(data_dir + "/top_10_largest_rtt_delta.txt", "w+") as f_out:
        largest_rtt_delta_list = []
        for src in range(len(ground_stations)):
            for dst in range(src + 1, len(ground_stations)):
                min_rtt_ns = np.min(rtt_list_per_pair[src][dst])
                max_rtt_ns = np.max(rtt_list_per_pair[src][dst])
                largest_rtt_delta_list.append((max_rtt_ns - min_rtt_ns, min_rtt_ns, max_rtt_ns, src, dst))
        largest_rtt_delta_list = sorted(largest_rtt_delta_list, reverse=True)
        f_out.write("LARGEST RTT DELTA TOP-10 WITHOUT DUPLICATE NODES\n")
        f_out.write("---------------------------------------------------------------\n")
        f_out.write("#      Pair           Delta (ms)   Min. RTT (ms)   Max. RTT (ms)\n")
        already_plotted_nodes = set()
        num_plotted = 0
        for i in range(len(largest_rtt_delta_list)):
            if largest_rtt_delta_list[i][3] not in already_plotted_nodes \
                    and largest_rtt_delta_list[i][4] not in already_plotted_nodes:
                f_out.write("%-3d    %-4d -> %4d   %-8.2f     %-8.2f        %-8.2f\n" % (
                    i + 1,
                    len(satellites) + largest_rtt_delta_list[i][3],
                    len(satellites) + largest_rtt_delta_list[i][4],
                    largest_rtt_delta_list[i][0] / 1e6,
                    largest_rtt_delta_list[i][1] / 1e6,
                    largest_rtt_delta_list[i][2] / 1e6,
                ))
                print_routes_and_rtt(base_output_dir, satellite_network_dir, dynamic_state_update_interval_ms,
                                     simulation_end_time_s, len(satellites) + largest_rtt_delta_list[i][3],
                                     len(satellites) + largest_rtt_delta_list[i][4], satgenpy_dir_with_ending_slash)
                already_plotted_nodes.add(largest_rtt_delta_list[i][3])
                already_plotted_nodes.add(largest_rtt_delta_list[i][4])
                num_plotted += 1
                if num_plotted >= 10:
                    break
        f_out.write("---------------------------------------------------------------\n")
        f_out.write("\n")

    # Most unreachable
    with open(data_dir + "/top_10_most_unreachable.txt", "w+") as f_out:
        most_unreachable_list = []
        for src in range(len(ground_stations)):
            for dst in range(src + 1, len(ground_stations)):
                most_unreachable_list.append((unreachable_per_pair[(src, dst)], src, dst))
        most_unreachable_list = sorted(most_unreachable_list, reverse=True)
        f_out.write("MOST UNREACHABLE DELTA TOP-10 WITHOUT DUPLICATE NODES\n")
        f_out.write("---------------------------------------\n")
        f_out.write("#      Pair           Times unreachable\n")
        already_plotted_nodes = set()
        num_plotted = 0
        for i in range(len(most_unreachable_list)):
            if most_unreachable_list[i][1] not in already_plotted_nodes \
                    and most_unreachable_list[i][2] not in already_plotted_nodes:
                f_out.write("%-3d    %-4d -> %4d   %d\n" % (
                    i + 1,
                    len(satellites) + most_unreachable_list[i][1],
                    len(satellites) + most_unreachable_list[i][2],
                    most_unreachable_list[i][0]
                ))
                print_routes_and_rtt(base_output_dir, satellite_network_dir, dynamic_state_update_interval_ms,
                                     simulation_end_time_s, len(satellites) + most_unreachable_list[i][1],
                                     len(satellites) + most_unreachable_list[i][2], satgenpy_dir_with_ending_slash)
                already_plotted_nodes.add(most_unreachable_list[i][1])
                already_plotted_nodes.add(most_unreachable_list[i][2])
                num_plotted += 1
                if num_plotted >= 10:
                    break
        f_out.write("---------------------------------------\n")
        f_out.write("\n")

    print("Done")
