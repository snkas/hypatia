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

from satgen.distance_tools import *
import networkx as nx
from astropy import units as u


def construct_graph_with_distances(epoch, time_since_epoch_ns, satellites, ground_stations, list_isls,
                                   max_gsl_length_m, max_isl_length_m):

    # Time
    time = epoch + time_since_epoch_ns * u.ns

    # Graph
    sat_net_graph_with_gs = nx.Graph()

    # ISLs
    for (a, b) in list_isls:

        # Only ISLs which are close enough are considered
        sat_distance_m = distance_m_between_satellites(satellites[a], satellites[b], str(epoch), str(time))
        if sat_distance_m <= max_isl_length_m:
            sat_net_graph_with_gs.add_edge(
                a, b, weight=sat_distance_m
            )

    # GSLs
    for ground_station in ground_stations:

        # Find satellites in range
        for sid in range(len(satellites)):
            distance_m = distance_m_ground_station_to_satellite(ground_station, satellites[sid], str(epoch), str(time))
            if distance_m <= max_gsl_length_m:
                sat_net_graph_with_gs.add_edge(len(satellites) + ground_station["gid"], sid, weight=distance_m)

    return sat_net_graph_with_gs


def compute_path_length_with_graph(path, graph):
    return sum_path_weights(augment_path_with_weights(path, graph))


def compute_path_length_without_graph(path, epoch, time_since_epoch_ns, satellites, ground_stations, list_isls,
                                      max_gsl_length_m, max_isl_length_m):

    # Time
    time = epoch + time_since_epoch_ns * u.ns

    # Go hop-by-hop and compute
    path_length_m = 0.0
    for i in range(1, len(path)):
        
        from_node_id = path[i - 1]
        to_node_id = path[i]
        
        # Satellite to satellite
        if from_node_id < len(satellites) and to_node_id < len(satellites):
            sat_distance_m = distance_m_between_satellites(
                satellites[from_node_id],
                satellites[to_node_id],
                str(epoch),
                str(time)
            )
            if sat_distance_m > max_isl_length_m \
                    or ((to_node_id, from_node_id) not in list_isls and (from_node_id, to_node_id) not in list_isls):
                raise ValueError("Invalid ISL hop")
            path_length_m += sat_distance_m

        # Ground station to satellite
        elif from_node_id >= len(satellites) and to_node_id < len(satellites):
            ground_station = ground_stations[from_node_id - len(satellites)]
            distance_m = distance_m_ground_station_to_satellite(
                ground_station,
                satellites[to_node_id],
                str(epoch),
                str(time)
            )
            if distance_m > max_gsl_length_m:
                raise ValueError("Invalid GSL hop from " + str(from_node_id) + " to " + str(to_node_id)
                                 + " (" + str(distance_m) + " larger than " + str(max_gsl_length_m) + ")")
            path_length_m += distance_m

        # Satellite to ground station
        elif from_node_id < len(satellites) and to_node_id >= len(satellites):
            ground_station = ground_stations[to_node_id - len(satellites)]
            distance_m = distance_m_ground_station_to_satellite(
                ground_station,
                satellites[from_node_id],
                str(epoch),
                str(time)
            )
            if distance_m > max_gsl_length_m:
                raise ValueError("Invalid GSL hop from " + str(from_node_id) + " to " + str(to_node_id)
                                 + " (" + str(distance_m) + " larger than " + str(max_gsl_length_m) + ")")
            path_length_m += distance_m

        else:
            raise ValueError("Hops between ground stations are not permitted: %d -> %d" % (from_node_id, to_node_id))

    return path_length_m


def get_path(src, dst, forward_state):

    if forward_state[(src, dst)] == -1:  # No path exists
        return None

    curr = src
    path = [src]
    while curr != dst:
        next_hop = forward_state[(curr, dst)]
        path.append(next_hop)
        curr = next_hop
    return path


def get_path_with_weights(src, dst, forward_state, sat_net_graph_with_gs):

    if forward_state[(src, dst)] == -1:  # No path exists
        return None

    curr = src
    path = []
    while curr != dst:
        next_hop = forward_state[(curr, dst)]
        w = sat_net_graph_with_gs.get_edge_data(curr, next_hop)["weight"]
        path.append((curr, next_hop, w))
        curr = next_hop
    return path


def augment_path_with_weights(path, sat_net_graph_with_gs):
    res = []
    for i in range(1, len(path)):
        res.append((path[i - 1], path[i], sat_net_graph_with_gs.get_edge_data(path[i - 1], path[i])["weight"]))
    return res


def sum_path_weights(weighted_path):
    res = 0.0
    for i in weighted_path:
        res += i[2]
    return res
