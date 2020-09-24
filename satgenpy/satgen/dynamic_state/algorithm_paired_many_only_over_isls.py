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


def algorithm_paired_many_only_over_isls(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        satellites,
        ground_stations,
        sat_net_graph_without_gs,
        ground_station_satellites_in_range,
        num_isls_per_sat,
        sat_neighbor_to_if,
        list_gsl_interfaces_info,
        prev_output,
        enable_verbose_logs
):
    """
    PAIRED-MANY ONLY OVER INTER-SATELLITE LINKS ALGORITHM

    "many"
    This algorithm assumes that every ground station has exactly 1 GSL interface and that
    every satellite has <number of ground stations> interfaces.

    "paired"
    Every ground station interfaced is paired to its nearest satellite on the interface corresponding to its gid.
    This pairing is only enforced in the forwarding state: from then on all packets will be sent to this satellite.

    "only_over_isls"
    It calculates a forwarding state, which is essentially a single shortest path.
    It only considers paths which go over the inter-satellite network, and does not make use of ground
    stations relay. This means that every path looks like:
    (src gs) - (sat) - (sat) - ... - (sat) - (dst gs)

    It can be used to answer questions about the effect of reordering caused by the satellite network
    on more complicated traffic matrices.
    """

    print("\nALGORITHM: PAIRED MANY ONLY OVER ISLS")

    # Check the graph
    if sat_net_graph_without_gs.number_of_nodes() != len(satellites):
        raise ValueError("Number of nodes in the graph does not match the number of satellites")
    for sid in range(len(satellites)):
        for n in sat_net_graph_without_gs.neighbors(sid):
            if n >= len(satellites):
                raise ValueError("Graph cannot contain satellite-to-ground-station links")

    # This algorithm only works:
    # (a) if the # of interfaces of satellites is exactly <number of ground stations>
    # (b) if the # of interfaces of ground stations is exactly 1
    # (c) if the aggregate interfaces bandwidth is the same for all nodes: 1.0
    for i in range(len(list_gsl_interfaces_info)):
        if i < len(satellites):
            if list_gsl_interfaces_info[i]["number_of_interfaces"] != len(ground_stations):
                raise ValueError("Satellites must have the same amount of GSL interfaces as ground stations exist")
        else:
            if list_gsl_interfaces_info[i]["number_of_interfaces"] != 1:
                raise ValueError("Ground stations must have exactly one interface")
        if list_gsl_interfaces_info[i]["aggregate_max_bandwidth"] != 1.0:
            raise ValueError("Aggregate max. bandwidth is not equal to 1.0")
    print("  > Interface conditions are met")

    # Free spots
    node_free_ifs = []
    node_if_count_list = list(map(
        lambda x: x["number_of_interfaces"],
        list_gsl_interfaces_info
    ))
    for i in range(len(satellites)):
        node_free_ifs.append(list(range(num_isls_per_sat[i], num_isls_per_sat[i] + node_if_count_list[i])))
    for i in range(len(satellites), len(satellites) + len(ground_stations)):
        node_free_ifs.append(list(range(node_if_count_list[i])))

    # The GSL mapping is very simple: each ground station has one interface, and connects it
    # in the forwarding state to the nearest satellite. Every satellite has <number of ground stations>
    # interfaces, so there is always a free interface spot on the satellite.
    gsl_mapping = {}
    frequency_satellite_chosen = [0] * len(satellites)
    for gid in range(len(ground_stations)):
        chosen_sid = -1
        best_distance_m = 1000000000000000
        for (distance_m, sid) in ground_station_satellites_in_range[gid]:
            if distance_m < best_distance_m:
                chosen_sid = sid
                best_distance_m = distance_m
        frequency_satellite_chosen[chosen_sid] += 1
        gsl_mapping[(gid, chosen_sid)] = {
            "distance_m": best_distance_m,
            "sat_if_idx": num_isls_per_sat[chosen_sid] + gid,
            "gs_if_idx": 0
        }
        node_free_ifs[chosen_sid].remove(gsl_mapping[(gid, chosen_sid)]["sat_if_idx"])
        node_free_ifs[len(satellites) + gid].remove(gsl_mapping[(gid, chosen_sid)]["gs_if_idx"])
    print("  > Created pairing of each ground station to its nearest satellite")

    # Set the assigned bandwidth
    for (gid, sid) in gsl_mapping:
        gsl_mapping[(gid, sid)]["bandwidth"] = 1.0 / float(frequency_satellite_chosen[sid])

    # Make a note of how many ground stations have switched
    if prev_output is not None:
        print("  > Ground stations that switched: %d" %
              len(set(gsl_mapping.keys()).difference(set(prev_output["gsl_mapping"].keys()))))

    #################################

    # Bandwidth state
    gsl_if_bandwidth_state = {}
    for (gid, sid) in gsl_mapping:
        gsl_if_bandwidth_state[(sid, gsl_mapping[(gid, sid)]["sat_if_idx"])] = gsl_mapping[(gid, sid)]["bandwidth"]
        gsl_if_bandwidth_state[(len(satellites) + gid, gsl_mapping[(gid, sid)]["gs_if_idx"])] = \
            gsl_mapping[(gid, sid)]["bandwidth"]
    for i in range(len(satellites) + len(ground_stations)):
        for f in node_free_ifs[i]:
            # Unmatched interfaces receive 100% of bandwidth of the interface to get rid of whatever remains
            # in its queue
            gsl_if_bandwidth_state[(i, f)] = list_gsl_interfaces_info[i]["aggregate_max_bandwidth"]

    # And now finally, we write the state:
    gsl_if_bandwidth_changes = 0
    output_filename = output_dynamic_state_dir + "/gsl_if_bandwidth_" + str(time_since_epoch_ns) + ".txt"
    print("  > Writing interface bandwidth state to: " + output_filename)
    with open(output_filename, "w+") as f_out:
        for (node_id, if_id) in gsl_if_bandwidth_state:

            # Only delta if have previous bandwidth state
            write = True
            if prev_output is not None:
                write = prev_output["gsl_if_bandwidth_state"][(node_id, if_id)] \
                        != gsl_if_bandwidth_state[(node_id, if_id)]

            # Write to file
            if write:
                f_out.write("%d,%d,%f\n" % (node_id, if_id, gsl_if_bandwidth_state[(node_id, if_id)]))
                gsl_if_bandwidth_changes += 1
    print("  > Interface bandwidth deltas... " + str(gsl_if_bandwidth_changes))

    #################################

    print("\nFORWARDING STATE")

    # Ground station
    ground_station_to_up_links = {}
    for gid in range(len(ground_stations)):
        ground_station_to_up_links[gid] = []
    list_gsls_node_ids = []
    for (gid, sid) in gsl_mapping:
        ground_station_to_up_links[gid].append({
            "sid": sid,
            "sat_if_idx": gsl_mapping[(gid, sid)]["sat_if_idx"],
            "gs_if_idx": gsl_mapping[(gid, sid)]["gs_if_idx"],
            "distance_m": gsl_mapping[(gid, sid)]["distance_m"],
        })
        list_gsls_node_ids.append((len(satellites) + gid, sid))

    # Calculate shortest paths
    print("  > Calculating Floyd-Warshall for ISL satellite network only")
    dist_sat_net_without_gs = nx.floyd_warshall_numpy(sat_net_graph_without_gs)

    # Forwarding state variable
    fstate = {}
    prev_fstate = None
    if prev_output is not None:
        prev_fstate = prev_output["fstate"]

    # Now write state to file for complete graph
    output_filename = output_dynamic_state_dir + "/fstate_" + str(time_since_epoch_ns) + ".txt"
    print("  > Writing forwarding state to: " + output_filename)
    with open(output_filename, "w+") as f_out:

        # Ground stations to ground stations
        # Choose the source satellite which promises the shortest path
        for src_gid in range(len(ground_stations)):
            for dst_gid in range(len(ground_stations)):
                if src_gid != dst_gid:
                    src_gs_node_id = len(satellites) + src_gid
                    dst_gs_node_id = len(satellites) + dst_gid

                    up_links = ground_station_to_up_links[src_gid]
                    down_links = ground_station_to_up_links[dst_gid]
                    cartesian_product_combinations = []
                    for i in range(len(up_links)):
                        a = up_links[i]
                        for b in down_links:
                            cartesian_product_combinations.append(
                                (
                                    a["distance_m"] + dist_sat_net_without_gs[(a["sid"], b["sid"])] + b["distance_m"],
                                    i
                                )
                            )
                    cartesian_product_combinations = sorted(cartesian_product_combinations)

                    # By default, if there is no satellite in ra1nge for one of the
                    # ground stations, it will be dropped (indicated by (-1, -1, -1) meaning no next hop, no interfaces)
                    decision = (-1, -1, -1)
                    if len(cartesian_product_combinations) > 0:
                        chosen = cartesian_product_combinations[0][1]
                        up_link = up_links[chosen]
                        decision = (up_link["sid"], up_link["gs_if_idx"], up_link["sat_if_idx"])

                    # Update forwarding state
                    if prev_fstate is None or prev_fstate[(src_gs_node_id, dst_gs_node_id)] != decision:
                        f_out.write("%d,%d,%d,%d,%d\n" % (src_gs_node_id, dst_gs_node_id,
                                                          decision[0], decision[1], decision[2]))
                    fstate[(src_gs_node_id, dst_gs_node_id)] = decision

        # Satellites to ground stations
        # Find the satellite which is the closest to the destination ground station
        for curr in range(len(satellites)):
            for dst_gid in range(len(ground_stations)):
                dst_gs_node_id = len(satellites) + dst_gid

                # The satellite this ground station
                down_links = ground_station_to_up_links[dst_gid]
                possibilities = []
                for j in range(len(down_links)):
                    b = down_links[j]
                    possibilities.append(
                        (
                            dist_sat_net_without_gs[(curr, b["sid"])] + b["distance_m"],
                            j
                        )
                    )
                possibilities = sorted(possibilities)

                # By default, if there is no satellite in range for the
                # destination ground station, it will be dropped (indicated by -1)
                decision = (-1, -1, -1)
                if len(possibilities) > 0:
                    chosen = possibilities[0][1]
                    down_link = down_links[chosen]

                    # If the current node is not that satellite, determine how to get to the satellite
                    if curr != down_link["sid"]:

                        # Among its neighbors, find the one which promises the
                        # lowest distance to reach the destination satellite
                        best_distance = 1000000000000000
                        for n in sat_net_graph_without_gs.neighbors(curr):
                            d = dist_sat_net_without_gs[(curr, n)] + dist_sat_net_without_gs[(n, down_link["sid"])]
                            if d < best_distance:
                                decision = (n, sat_neighbor_to_if[(curr, n)], sat_neighbor_to_if[(n, curr)])
                                best_distance = d

                    else:
                        # This is the destination satellite, as such the next hop is the ground station itself
                        decision = (dst_gs_node_id, down_link["sat_if_idx"], down_link["gs_if_idx"])

                # Update forwarding state
                if prev_fstate is None or prev_fstate[(curr, dst_gs_node_id)] != decision:
                    f_out.write("%d,%d,%d,%d,%d\n" % (curr, dst_gs_node_id, decision[0], decision[1], decision[2]))
                fstate[(curr, dst_gs_node_id)] = decision

    print("")

    return {
        "fstate": fstate,
        "gsl_mapping": gsl_mapping,
        "gsl_if_bandwidth_state": gsl_if_bandwidth_state
    }
