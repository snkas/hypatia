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

import networkx as nx
import math


def algorithm_free_one_only_gs_relays(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        satellites,
        ground_stations,
        sat_net_graph_only_gs,
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

    As such, it should only be used to answer questions of:
    (a) reachability / RTT over time,
    (b) accurate bandwidth if only having a single communication pair.

    If this is not respected, it is e.g. possible that a ground station can only send at
    for example 10G, but all satellites in range can send to him in
    be used for questions of a single pair of communicating
    """

    if enable_verbose_logs:
        print("\nALGORITHM: FREE ONE ONLY GS RELAYS")

    # For this algorithm to function, there cannot be any ISLs
    for sid in range(len(num_isls_per_sat)):
        if num_isls_per_sat[sid] > 0:
            raise ValueError("No satellite ISLs are permitted for this algorithm. Violated for satellite %d" % sid)

    # Check the graph
    for sid in range(len(satellites)):
        for n in sat_net_graph_only_gs.neighbors(sid):
            if n < len(satellites):
                raise ValueError("Graph cannot contain inter-satellite links")
    for gid in range(len(ground_stations)):
        for n in sat_net_graph_only_gs.neighbors(len(satellites) + gid):
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
                f_out.write("%d,%d,%f\n"
                            % (node_id, num_isls_per_sat[node_id],
                               list_gsl_interfaces_info[node_id]["aggregate_max_bandwidth"]))
            for node_id in range(len(satellites), len(satellites) + len(ground_stations)):
                f_out.write("%d,%d,%f\n"
                            % (node_id, 0, list_gsl_interfaces_info[node_id]["aggregate_max_bandwidth"]))

    #################################
    # FORWARDING STATE
    #

    # Calculate shortest paths
    if enable_verbose_logs:
        print("  > Calculating Floyd-Warshall for graph only with ground-station relays")
    dist_sat_net = nx.floyd_warshall_numpy(sat_net_graph_only_gs)

    # Forwarding state
    fstate = {}
    prev_fstate = None
    if prev_output is not None:
        prev_fstate = prev_output["fstate"]

    # Now write state to file for complete graph
    output_filename = output_dynamic_state_dir + "/fstate_" + str(time_since_epoch_ns) + ".txt"
    if enable_verbose_logs:
        print("  > Writing forwarding state to: " + output_filename)
    with open(output_filename, "w+") as f_out:

        # Satellites and ground stations to ground stations
        for current_node_id in range(len(satellites) + len(ground_stations)):
            for dst_gs in range(len(ground_stations)):
                dst_gs_node_id = len(satellites) + dst_gs

                if current_node_id != dst_gs_node_id:

                    # Among its neighbors, find the one which promises the
                    # lowest distance to reach the destination satellite
                    next_hop_decision = (-1, -1, -1)
                    best_dist = 1000000000000000
                    for n in sat_net_graph_only_gs.neighbors(current_node_id):

                        if math.isinf(dist_sat_net[(current_node_id, n)]):
                            raise ValueError("Neighbor cannot be unreachable")

                        if not math.isinf(dist_sat_net[(n, dst_gs_node_id)]) and \
                           dist_sat_net[(current_node_id, n)] + dist_sat_net[(n, dst_gs_node_id)] < best_dist:

                            # Interface identifiers
                            if current_node_id >= len(satellites) and n < len(satellites):  # GS to satellite
                                my_if = 0
                                next_hop_if = num_isls_per_sat[n]  # The last interface is the one towards GSs

                            elif current_node_id < len(satellites) and n >= len(satellites):  # Satellite to GS
                                my_if = num_isls_per_sat[current_node_id]  # The last interface is the one towards GSs
                                next_hop_if = 0

                            else:  # Satellite to satellite, or GS to GS
                                raise ValueError("sat-sat or gs-gs link should not exist")

                            # Write the next-hop decision
                            next_hop_decision = (
                                n,            # Next-hop node identifier
                                my_if,        # My outgoing interface id
                                next_hop_if   # Next-hop incoming interface id
                            )

                            # Update best distance found
                            best_dist = dist_sat_net[(current_node_id, n)] + dist_sat_net[(n, dst_gs_node_id)]

                    # Write to forwarding state
                    if not prev_fstate or prev_fstate[(current_node_id, dst_gs_node_id)] != next_hop_decision:
                        f_out.write("%d,%d,%d,%d,%d\n" % (
                            current_node_id,
                            dst_gs_node_id,
                            next_hop_decision[0],
                            next_hop_decision[1],
                            next_hop_decision[2]
                        ))
                    fstate[(current_node_id, dst_gs_node_id)] = next_hop_decision

    if enable_verbose_logs:
        print("")

    return {
        "fstate": fstate
    }
