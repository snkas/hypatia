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


def algorithm_free_one_only_gs_relays(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        satellites,
        ground_stations,
        sat_net_graph_only_satellites_with_isls,
        num_isls_per_sat,
        list_gsl_interfaces_info,
        prev_output,
        enable_verbose_logs
):
    """
    FREE-ONE ONLY OVER GROUND STATION RELAYS ALGORITHM

    "one"
    This algorithm assumes that every satellite and ground station has exactly 1 GSL interface.

    "free"
    This 1 interface is bound to a maximum outgoing bandwidth, but can send to any other
    GSL interface (well, satellite -> ground-station, and ground-station -> satellite) in
    range. ("free") There is no reciprocation of the bandwidth asserted.

    "only_gs_relays"
    It calculates a forwarding state, which is essentially a single shortest path.
    It only considers paths which alternative ground station and satellite.
    This is called only ground stations relays:
    (src gs) - (sat) - (intermediary gs) - (sat) - (intermediary gs) - ... - (sat) - (dst gs)

    """

    if enable_verbose_logs:
        print("\nALGORITHM: FREE ONE ONLY GS RELAYS")

    # For this algorithm to function, there cannot be any ISLs
    for sid in range(len(num_isls_per_sat)):
        if num_isls_per_sat[sid] > 0:
            raise ValueError("No satellite ISLs are permitted for this algorithm. Violated for satellite %d" % sid)

    # Check the graph
    for sid in range(len(satellites)):
        for n in sat_net_graph_only_satellites_with_isls.neighbors(sid):
            if n < len(satellites):
                raise ValueError("Graph cannot contain inter-satellite links")
    for gid in range(len(ground_stations)):
        for n in sat_net_graph_only_satellites_with_isls.neighbors(len(satellites) + gid):
            if n >= len(satellites):
                raise ValueError("Graph cannot contain inter-ground-station links")

    #################################
    # BANDWIDTH STATE
    #

    # There is only one GSL interface for each node (pre-condition), which as-such will get the entire bandwidth
    output_filename = output_dynamic_state_dir + "/gsl_if_bandwidth_" + str(time_since_epoch_ns) + ".txt"
    if enable_verbose_logs:
        print("  > Writing interface bandwidth state to: " + output_filename)
    with open(output_filename, "w+") as f_out:
        if time_since_epoch_ns == 0:
            for node_id in range(len(satellites)):
                f_out.write("%d,%d,%f\n" % (
                    node_id,
                    num_isls_per_sat[node_id],
                    list_gsl_interfaces_info[node_id]["aggregate_max_bandwidth"]
                ))
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
    gid_to_sat_gsl_if_idx = [0] * len(ground_stations)  # (Only one GSL interface per satellite, so the first)

    # Forwarding state using shortest paths
    fstate = calculate_fstate_shortest_path_with_gs_relaying(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        len(satellites),
        len(ground_stations),
        sat_net_graph_only_satellites_with_isls,
        num_isls_per_sat,
        gid_to_sat_gsl_if_idx,
        {},
        prev_fstate,
        enable_verbose_logs
    )

    if enable_verbose_logs:
        print("")

    return {
        "fstate": fstate
    }
