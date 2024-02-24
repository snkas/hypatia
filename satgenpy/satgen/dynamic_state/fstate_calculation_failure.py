import math
import networkx as nx


def calculate_fstate_shortest_path_without_gs_relaying_failure(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        active_satellite_ids,
        active_ground_station_ids,
        sat_net_graph_only_satellites_with_isls,
        num_isls_per_sat,
        gid_to_sat_gsl_if_idx,
        ground_station_satellites_in_range_candidates,
        sat_neighbor_to_if,
        prev_fstate,
        enable_verbose_logs
):

    # Calculate shortest path distances
    if enable_verbose_logs:
        print("  > Calculating Floyd-Warshall for graph without ground-station relays")
    # (Note: Numpy has a deprecation warning here because of how networkx uses matrices)
    distance_map = nx.floyd_warshall_numpy(sat_net_graph_only_satellites_with_isls)

    # Forwarding state
    fstate = {}

    # Now write state to file for complete graph
    output_filename = output_dynamic_state_dir + "/fstate_" + str(time_since_epoch_ns) + ".txt"
    if enable_verbose_logs:
        print("  > Writing forwarding state to: " + output_filename)
    with open(output_filename, "w+") as f_out:

        # Satellites to ground stations
        # From the satellites attached to the destination ground station,
        # select the one which promises the shortest path to the destination ground station (getting there + last hop)
        dist_satellite_to_ground_station = {}
        for curr in active_satellite_ids:
            for dst_gs_node_id in active_ground_station_ids: # dst_gs_node_gid is already offset by 1584
                dst_gid = dst_gs_node_id - 1584
                # Among the satellites in range of the destination ground station,
                # find the one which promises the shortest distance
                possible_dst_sats = ground_station_satellites_in_range_candidates[dst_gid]
                if isinstance(possible_dst_sats, int):
                    continue
                possibilities = []
                for b in possible_dst_sats:
                    if not math.isinf(distance_map[(curr, b[1])]):  # Must be reachable
                        possibilities.append(
                            (
                                distance_map[(curr, b[1])] + b[0],
                                b[1]
                            )
                        )
                possibilities = list(sorted(possibilities))

                # By default, if there is no satellite in range for the
                # destination ground station, it will be dropped (indicated by -1)
                next_hop_decision = (-1, -1, -1)
                distance_to_ground_station_m = float("inf")
                if len(possibilities) > 0:
                    dst_sat = possibilities[0][1]
                    distance_to_ground_station_m = possibilities[0][0]

                    # If the current node is not that satellite, determine how to get to the satellite
                    if curr != dst_sat:

                        # Among its neighbors, find the one which promises the
                        # lowest distance to reach the destination satellite
                        best_distance_m = 1000000000000000
                        for neighbor_id in sat_net_graph_only_satellites_with_isls.neighbors(curr):

                            distance_m = (
                                    sat_net_graph_only_satellites_with_isls.edges[(curr, neighbor_id)]["weight"]
                                    +
                                    distance_map[(neighbor_id, dst_sat)]
                            )
                            if distance_m < best_distance_m:
                                next_hop_decision = (
                                    neighbor_id,
                                    sat_neighbor_to_if[(curr, neighbor_id)],
                                    sat_neighbor_to_if[(neighbor_id, curr)]
                                )
                                best_distance_m = distance_m

                    else:
                        # This is the destination satellite, as such the next hop is the ground station itself
                        next_hop_decision = (
                            dst_gs_node_id,
                            num_isls_per_sat[dst_sat] + gid_to_sat_gsl_if_idx[dst_gid],
                            0
                        )

                # In any case, save the distance of the satellite to the ground station to re-use
                # when we calculate ground station to ground station forwarding
                dist_satellite_to_ground_station[(curr, dst_gs_node_id)] = distance_to_ground_station_m
                # Write to forwarding state
                # curr is the active satellite id 
                if not prev_fstate or prev_fstate.get((curr, dst_gs_node_id), None) != next_hop_decision:                    
                    f_out.write("%d,%d,%d,%d,%d\n" % (
                        curr,
                        dst_gs_node_id,
                        next_hop_decision[0],
                        next_hop_decision[1],
                        next_hop_decision[2]
                    ))
                fstate[(curr, dst_gs_node_id)] = next_hop_decision

        # Ground stations to ground stations
        # Choose the source satellite which promises the shortest path
        for src_gs_node_id in active_ground_station_ids:
            for dst_gs_node_id in active_ground_station_ids:
                if src_gs_node_id != dst_gs_node_id:
                    src_gid = src_gs_node_id - 1584
                    dst_gid = dst_gs_node_id - 1584

                    # Among the satellites in range of the source ground station,
                    # find the one which promises the shortest distance
                    possible_src_sats = ground_station_satellites_in_range_candidates[src_gid]
                    if isinstance(possible_src_sats, int):
                        continue
                    possibilities = []
                    for a in possible_src_sats:
                        best_distance_offered_m = dist_satellite_to_ground_station[(a[1], dst_gs_node_id)]
                        if not math.isinf(best_distance_offered_m):
                            possibilities.append(
                                (
                                    a[0] + best_distance_offered_m,
                                    a[1]
                                )
                            )
                    possibilities = sorted(possibilities)

                    # By default, if there is no satellite in range for one of the
                    # ground stations, it will be dropped (indicated by -1)
                    next_hop_decision = (-1, -1, -1)
                    if len(possibilities) > 0:
                        src_sat_id = possibilities[0][1]
                        next_hop_decision = (
                            src_sat_id,
                            0,
                            num_isls_per_sat[src_sat_id] + gid_to_sat_gsl_if_idx[src_gid]
                        )

                    # Update forwarding state
                    if not prev_fstate or prev_fstate.get((src_gs_node_id, dst_gs_node_id), None) != next_hop_decision:                        
                        f_out.write("%d,%d,%d,%d,%d\n" % (
                            src_gs_node_id,
                            dst_gs_node_id,
                            next_hop_decision[0],
                            next_hop_decision[1],
                            next_hop_decision[2]
                        ))
                    fstate[(src_gs_node_id, dst_gs_node_id)] = next_hop_decision

    # Finally return result
    return fstate
