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

from .fstate_calculation import *


def algorithm_free_gs_one_sat_many_only_over_isls(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        satellites,
        ground_stations,
        sat_net_graph_only_satellites_with_isls,
        ground_station_satellites_in_range,
        num_isls_per_sat,
        sat_neighbor_to_if,
        list_gsl_interfaces_info,
        prev_output,
        enable_verbose_logs
):
    """
    FREE GROUND STATION (ONE) SATELLITE (MANY) OVER INTER-SATELLITE LINKS ALGORITHM

    "gs_one"
    This algorithm assumes that every ground station has exactly 1 GSL interface.

    "sat_many"
    This algorithm assumes that every satellite has exactly <number of ground stations> GSL interfaces.

    "free"
    The 1 GSL interface is free to whichever satellite it wants.
    A GSL interface in a satellite is only used to send to one ground station.
    This means effectively that the bandwidth a ground station can receive is the amount it can send at, always.

    "only_over_isls"
    It calculates a forwarding state, which is essentially a single shortest path.
    It only considers paths which go over the inter-satellite network, and does not make use of ground
    stations relay. This means that every path looks like:
    (src gs) - (sat) - (sat) - ... - (sat) - (dst gs)
    """

    if enable_verbose_logs:
        print("\nALGORITHM: FREE GROUND STATION ONE SATELLITE MANY ONLY OVER ISLS")

    # Check the graph
    if sat_net_graph_only_satellites_with_isls.number_of_nodes() != len(satellites):
        raise ValueError("Number of nodes in the graph does not match the number of satellites")
    for sid in range(len(satellites)):
        for n in sat_net_graph_only_satellites_with_isls.neighbors(sid):
            if n >= len(satellites):
                raise ValueError("Graph cannot contain satellite-to-ground-station links")

    # This algorithm only works:
    # (a) if the # of interfaces of satellites is exactly <number of ground stations>
    # (b) if the # of interfaces of ground stations is exactly 1
    # (c) if the aggregate interfaces bandwidth is 1.0 for ground stations
    # (d) if the aggregate interfaces bandwidth is <number of ground stations> * 1.0 for all satellites
    for i in range(len(list_gsl_interfaces_info)):
        if i < len(satellites):
            if list_gsl_interfaces_info[i]["number_of_interfaces"] != len(ground_stations):
                raise ValueError("Satellites must have the same amount of GSL interfaces as ground stations exist")
            if list_gsl_interfaces_info[i]["aggregate_max_bandwidth"] != len(ground_stations):
                raise ValueError("Satellite aggregate max. bandwidth is not equal to 1.0 times number of GSs")
        else:
            if list_gsl_interfaces_info[i]["number_of_interfaces"] != 1:
                raise ValueError("Ground stations must have exactly one interface")
            if list_gsl_interfaces_info[i]["aggregate_max_bandwidth"] != 1.0:
                raise ValueError("Ground station aggregate max. bandwidth is not equal to 1.0")
    if enable_verbose_logs:
        print("  > Interface conditions are met")

    #################################
    # BANDWIDTH STATE
    #

    # There is one GSL interface per ground station, and <# of GSs> interfaces per satellite
    output_filename = output_dynamic_state_dir + "/gsl_if_bandwidth_" + str(time_since_epoch_ns) + ".txt"
    if enable_verbose_logs:
        print("  > Writing interface bandwidth state to: " + output_filename)
    with open(output_filename, "w+") as f_out:
        if time_since_epoch_ns == 0:

            # Satellite have <# of GSs> interfaces besides their ISL interfaces
            for node_id in range(len(satellites)):
                for i in range(list_gsl_interfaces_info[node_id]["number_of_interfaces"]):
                    f_out.write("%d,%d,%f\n" % (
                        node_id,
                        num_isls_per_sat[node_id] + i,
                        list_gsl_interfaces_info[node_id]["aggregate_max_bandwidth"]
                        / float(list_gsl_interfaces_info[node_id]["number_of_interfaces"])
                    ))

            # Ground stations have one GSL interface: 0
            for node_id in range(len(satellites), len(satellites) + len(ground_stations)):
                f_out.write("%d,%d,%f\n" % (
                    node_id,
                    0,
                    list_gsl_interfaces_info[node_id]["aggregate_max_bandwidth"]
                 ))

    #################################
    # FORWARDING STATE
    #

    # Previous forwarding state (to only write delta)
    prev_fstate = None
    if prev_output is not None:
        prev_fstate = prev_output["fstate"]

    # GID to satellite GSL interface index
    # Each ground station has a GSL interface on every
    # satellite allocated only for itself
    gid_to_sat_gsl_if_idx = list(range(len(ground_stations)))

    # Forwarding state using shortest paths
    fstate = calculate_fstate_shortest_path_without_gs_relaying(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        len(satellites),
        len(ground_stations),
        sat_net_graph_only_satellites_with_isls,
        num_isls_per_sat,
        gid_to_sat_gsl_if_idx,
        ground_station_satellites_in_range,
        sat_neighbor_to_if,
        prev_fstate,
        enable_verbose_logs
    )

    if enable_verbose_logs:
        print("")

    return {
        "fstate": fstate
    }
