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

    ###########################################################
    # Select the nearest satellite for each ground station
    #

    # Keep track of which satellite GSL interfaces (it has <n of ground stations> interfaces) are paired
    # because it is the closest satellite to a ground station
    satellite_gsl_ifs_paired = []
    for sid in range(len(satellites)):
        satellite_gsl_ifs_paired.append([])

    # Go over each ground station
    ground_station_satellites_in_range_select_one_at_most = []
    for gid in range(len(ground_stations)):

        # Find the closest satellite
        chosen_sid = -1
        best_distance_m = 1000000000000000
        for (distance_m, sid) in ground_station_satellites_in_range[gid]:
            if distance_m < best_distance_m:
                chosen_sid = sid
                best_distance_m = distance_m

        # It is possible that a ground station does not have a single satellite in-range
        if chosen_sid == -1:
            ground_station_satellites_in_range_select_one_at_most.append([])
        else:
            ground_station_satellites_in_range_select_one_at_most.append([(best_distance_m, chosen_sid)])
            satellite_gsl_ifs_paired[chosen_sid].append(gid)

    ##################################################
    # Determine the new GSL interface bandwidth state
    #

    # Bandwidth state
    gsl_if_bandwidth_state = {}

    # For the satellite
    for sid in range(len(satellites)):

        # The paired GSL interfaces share the total bandwidth
        # The other ones are not in use, but still need to flush out existing packets
        satellite_frequency_chosen = len(satellite_gsl_ifs_paired[sid])
        for gsl_if_idx in range(len(ground_stations)):

            # If it is paired, it gets its fair share
            if gsl_if_idx in satellite_gsl_ifs_paired[sid]:
                gsl_if_bandwidth_state[(sid, num_isls_per_sat[sid] + gsl_if_idx)] = (
                        1.0 / float(satellite_frequency_chosen)
                )

            # Else, it get the full bandwidth to get rid of the packets in it (it can also be kept, but then
            # you cannot parallelize this generation process)
            else:
                gsl_if_bandwidth_state[(sid, num_isls_per_sat[sid] + gsl_if_idx)] = 1.0

    # For the ground stations, the same principle applies
    for gid in range(len(ground_stations)):

        # Check if it is paired
        if len(ground_station_satellites_in_range_select_one_at_most[gid]) == 1:

            # Find the satellite it is paired to, and then only get its fair share
            paired_satellite_id = ground_station_satellites_in_range_select_one_at_most[gid][0][1]
            satellite_frequency_chosen = len(satellite_gsl_ifs_paired[paired_satellite_id])
            gsl_if_bandwidth_state[(len(satellites) + gid, 0)] = 1.0 / float(satellite_frequency_chosen)

        # If not paired, flush out the packets (it can also be kept, but then
        # you cannot parallelize this generation process)
        else:
            gsl_if_bandwidth_state[(len(satellites) + gid, 0)] = 1.0

    ######################################################
    # Write the new GSL interface bandwidth state (delta)
    #

    # Previous GSL interface bandwidth state (to only write delta)
    prev_gsl_if_bandwidth_state = None
    if prev_output is not None:
        prev_gsl_if_bandwidth_state = prev_output["gsl_if_bandwidth_state"]

    output_filename = output_dynamic_state_dir + "/gsl_if_bandwidth_" + str(time_since_epoch_ns) + ".txt"
    print("  > Writing interface bandwidth state to: " + output_filename)
    with open(output_filename, "w+") as f_out:
        for (node_id, if_id) in gsl_if_bandwidth_state:

            # Only delta if have previous bandwidth state
            if (
                    prev_gsl_if_bandwidth_state is None
                    or
                    prev_gsl_if_bandwidth_state[(node_id, if_id)] != gsl_if_bandwidth_state[(node_id, if_id)]
            ):
                f_out.write("%d,%d,%f\n" % (
                    node_id,
                    if_id,
                    gsl_if_bandwidth_state[(node_id, if_id)]
                ))

    #################################

    print("\nFORWARDING STATE")

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
        sat_net_graph_without_gs,
        num_isls_per_sat,
        gid_to_sat_gsl_if_idx,
        ground_station_satellites_in_range_select_one_at_most,
        sat_neighbor_to_if,
        prev_fstate,
        enable_verbose_logs
    )

    print("")

    return {
        "fstate": fstate,
        "gsl_if_bandwidth_state": gsl_if_bandwidth_state
    }
