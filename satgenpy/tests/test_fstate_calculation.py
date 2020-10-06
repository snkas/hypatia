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

import exputil
import unittest
from satgen.dynamic_state.fstate_calculation import *


def calculate_fstate_for(
        num_satellites,
        num_ground_stations,
        edges
):
    local_shell = exputil.LocalShell()

    sat_net_graph_only_isls = nx.Graph()
    sat_net_graph_only_gsls = nx.Graph()
    sat_net_graph_complete = nx.Graph()

    # Nodes
    ground_station_satellites_in_range = []
    for i in range(num_satellites):
        sat_net_graph_only_isls.add_node(i)
        sat_net_graph_only_gsls.add_node(i)
        sat_net_graph_complete.add_node(i)
    for i in range(num_satellites, num_satellites + num_ground_stations):
        sat_net_graph_only_gsls.add_node(i)
        sat_net_graph_complete.add_node(i)
        ground_station_satellites_in_range.append([])

    # Edges
    num_isls_per_sat = [0] * num_satellites
    sat_neighbor_to_if = {}
    for e in edges:
        if e[0] < num_satellites and e[1] < num_satellites:
            sat_net_graph_only_isls.add_edge(
                e[0], e[1], weight=e[2]
            )
            sat_net_graph_complete.add_edge(
                e[0], e[1], weight=e[2]
            )
            sat_neighbor_to_if[(e[0], e[1])] = num_isls_per_sat[e[0]]
            sat_neighbor_to_if[(e[1], e[0])] = num_isls_per_sat[e[1]]
            num_isls_per_sat[e[0]] += 1
            num_isls_per_sat[e[1]] += 1
        if e[0] >= num_satellites or e[1] >= num_satellites:
            sat_net_graph_only_gsls.add_edge(
                e[0], e[1], weight=e[2]
            )
            sat_net_graph_complete.add_edge(
                e[0], e[1], weight=e[2]
            )
            ground_station_satellites_in_range[max(e[0], e[1]) - num_satellites].append(
                (e[2], min(e[0], e[1]))
            )

    # GS relays only does not have ISLs
    num_isls_per_sat_for_only_gs_relays = [0] * num_satellites

    # Finally, GID to the satellite GSL interface index it communicates to on each satellite
    gid_to_sat_gsl_if_idx = list(range(num_ground_stations))

    # Output directory
    temp_dir = "temp_fstate_calculation_test"
    local_shell.make_full_dir(temp_dir)
    output_dynamic_state_dir = temp_dir
    time_since_epoch_ns = 0

    # Now let's call it
    prev_fstate = None
    enable_verbose_logs = True

    # Return all three
    result = {
        "without_gs_relays": calculate_fstate_shortest_path_without_gs_relaying(
            output_dynamic_state_dir,
            time_since_epoch_ns,
            num_satellites,
            num_ground_stations,
            sat_net_graph_only_isls,
            num_isls_per_sat,
            gid_to_sat_gsl_if_idx,
            ground_station_satellites_in_range,
            sat_neighbor_to_if,
            prev_fstate,
            enable_verbose_logs
        ),
        "only_gs_relays": calculate_fstate_shortest_path_with_gs_relaying(
            output_dynamic_state_dir,
            time_since_epoch_ns,
            num_satellites,
            num_ground_stations,
            sat_net_graph_only_gsls,
            num_isls_per_sat_for_only_gs_relays,
            gid_to_sat_gsl_if_idx,
            sat_neighbor_to_if,
            prev_fstate,
            enable_verbose_logs
        ),
        "combined": calculate_fstate_shortest_path_with_gs_relaying(
            output_dynamic_state_dir,
            time_since_epoch_ns,
            num_satellites,
            num_ground_stations,
            sat_net_graph_complete,
            num_isls_per_sat,
            gid_to_sat_gsl_if_idx,
            sat_neighbor_to_if,
            prev_fstate,
            enable_verbose_logs
        )
    }

    # Remove the temporary directory afterwards
    local_shell.remove_force_recursive(temp_dir)

    return result


class TestFstateCalculation(unittest.TestCase):

    def test_one_sat_two_gs(self):

        #   0
        #  / \
        # 1   2

        num_satellites = 1
        num_ground_stations = 2

        edges = [
            (0, 1, 1000), (0, 2, 1000)
        ]

        output = calculate_fstate_for(num_satellites, num_ground_stations, edges)

        self.assertEqual(output["without_gs_relays"][(0, 1)], (1, 0, 0))
        self.assertEqual(output["without_gs_relays"][(0, 2)], (2, 1, 0))
        self.assertEqual(output["without_gs_relays"][(1, 2)], (0, 0, 0))
        self.assertEqual(output["without_gs_relays"][(2, 1)], (0, 0, 1))

        self.assertEqual(output["only_gs_relays"][(0, 1)], (1, 0, 0))
        self.assertEqual(output["only_gs_relays"][(0, 2)], (2, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 2)], (0, 0, 0))
        self.assertEqual(output["only_gs_relays"][(2, 1)], (0, 0, 1))

        self.assertEqual(output["combined"][(0, 1)], (1, 0, 0))
        self.assertEqual(output["combined"][(0, 2)], (2, 1, 0))
        self.assertEqual(output["combined"][(1, 2)], (0, 0, 0))
        self.assertEqual(output["combined"][(2, 1)], (0, 0, 1))

    def test_two_sat_two_gs(self):

        #   0 -- 1
        #  /      \
        # 2       3

        num_satellites = 2
        num_ground_stations = 2

        edges = [
            (0, 1, 1000),
            (0, 2, 1000),
            (1, 3, 1000)
        ]

        output = calculate_fstate_for(num_satellites, num_ground_stations, edges)

        self.assertEqual(output["without_gs_relays"][(0, 2)], (2, 1, 0))
        self.assertEqual(output["without_gs_relays"][(0, 3)], (1, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 2)], (0, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 3)], (3, 2, 0))
        self.assertEqual(output["without_gs_relays"][(2, 3)], (0, 0, 1))
        self.assertEqual(output["without_gs_relays"][(3, 2)], (1, 0, 2))

        self.assertEqual(output["only_gs_relays"][(0, 2)], (2, 0, 0))
        self.assertEqual(output["only_gs_relays"][(0, 3)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(1, 2)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(1, 3)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(2, 3)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(3, 2)], (-1, -1, -1))

        self.assertEqual(output["combined"][(0, 2)], (2, 1, 0))
        self.assertEqual(output["combined"][(0, 3)], (1, 0, 0))
        self.assertEqual(output["combined"][(1, 2)], (0, 0, 0))
        self.assertEqual(output["combined"][(1, 3)], (3, 2, 0))
        self.assertEqual(output["combined"][(2, 3)], (0, 0, 1))
        self.assertEqual(output["combined"][(3, 2)], (1, 0, 2))

    def test_two_sat_three_gs(self):

        #   0 ----- 1
        #  /   \ /   \
        # 2     3    4

        num_satellites = 2
        num_ground_stations = 3

        edges = [
            (0, 1, 1000),
            (0, 2, 500),
            (0, 3, 200),
            (1, 3, 100),
            (1, 4, 400)
        ]

        output = calculate_fstate_for(num_satellites, num_ground_stations, edges)

        self.assertEqual(output["without_gs_relays"][(0, 2)], (2, 1, 0))
        self.assertEqual(output["without_gs_relays"][(0, 3)], (3, 2, 0))
        self.assertEqual(output["without_gs_relays"][(0, 4)], (1, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 2)], (0, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 3)], (3, 2, 0))
        self.assertEqual(output["without_gs_relays"][(1, 4)], (4, 3, 0))
        self.assertEqual(output["without_gs_relays"][(2, 3)], (0, 0, 1))
        self.assertEqual(output["without_gs_relays"][(2, 4)], (0, 0, 1))
        self.assertEqual(output["without_gs_relays"][(3, 2)], (0, 0, 2))
        self.assertEqual(output["without_gs_relays"][(3, 4)], (1, 0, 2))
        self.assertEqual(output["without_gs_relays"][(4, 2)], (1, 0, 3))
        self.assertEqual(output["without_gs_relays"][(4, 3)], (1, 0, 3))

        self.assertEqual(output["only_gs_relays"][(0, 2)], (2, 0, 0))
        self.assertEqual(output["only_gs_relays"][(0, 3)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(0, 4)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 2)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 3)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 4)], (4, 2, 0))
        self.assertEqual(output["only_gs_relays"][(2, 3)], (0, 0, 0))
        self.assertEqual(output["only_gs_relays"][(2, 4)], (0, 0, 0))
        self.assertEqual(output["only_gs_relays"][(3, 2)], (0, 0, 1))
        self.assertEqual(output["only_gs_relays"][(3, 4)], (1, 0, 1))
        self.assertEqual(output["only_gs_relays"][(4, 2)], (1, 0, 2))
        self.assertEqual(output["only_gs_relays"][(4, 3)], (1, 0, 2))

        self.assertEqual(output["combined"][(0, 2)], (2, 1, 0))
        self.assertEqual(output["combined"][(0, 3)], (3, 2, 0))
        self.assertEqual(output["combined"][(0, 4)], (3, 2, 0))
        self.assertEqual(output["combined"][(1, 2)], (3, 2, 0))
        self.assertEqual(output["combined"][(1, 3)], (3, 2, 0))
        self.assertEqual(output["combined"][(1, 4)], (4, 3, 0))
        self.assertEqual(output["combined"][(2, 3)], (0, 0, 1))
        self.assertEqual(output["combined"][(2, 4)], (0, 0, 1))
        self.assertEqual(output["combined"][(3, 2)], (0, 0, 2))
        self.assertEqual(output["combined"][(3, 4)], (1, 0, 2))
        self.assertEqual(output["combined"][(4, 2)], (1, 0, 3))
        self.assertEqual(output["combined"][(4, 3)], (1, 0, 3))

    def test_five_sat_five_gs(self):

        #
        #  7 --  0       1 -- 6
        #      / |       | \
        #    8    3     4   9
        #    |      \  /    |
        #    +------ 2 -----+
        #            |
        #            5
        #

        num_satellites = 5
        num_ground_stations = 5

        edges = [
            (0, 3, 600),
            (0, 7, 500),
            (0, 8, 600),
            (1, 4, 300),
            (1, 6, 500),
            (1, 9, 300),
            (2, 3, 400),
            (2, 4, 400),
            (2, 5, 500),
            (2, 8, 200),
            (2, 9, 500),
        ]

        output = calculate_fstate_for(num_satellites, num_ground_stations, edges)

        #
        #  7 --  0       1 -- 6
        #      / |       | \
        #    8    3     4   9
        #    |      \  /    |
        #    +------ 2 -----+
        #            |
        #            5
        #

        self.assertEqual(output["without_gs_relays"][(0, 5)], (3, 0, 0))
        self.assertEqual(output["without_gs_relays"][(0, 6)], (3, 0, 0))
        self.assertEqual(output["without_gs_relays"][(0, 7)], (7, 3, 0))
        self.assertEqual(output["without_gs_relays"][(0, 8)], (8, 4, 0))
        self.assertEqual(output["without_gs_relays"][(0, 9)], (3, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 5)], (4, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 6)], (6, 2, 0))
        self.assertEqual(output["without_gs_relays"][(1, 7)], (4, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 8)], (4, 0, 0))
        self.assertEqual(output["without_gs_relays"][(1, 9)], (9, 5, 0))
        self.assertEqual(output["without_gs_relays"][(2, 5)], (5, 2, 0))
        self.assertEqual(output["without_gs_relays"][(2, 6)], (4, 1, 1))
        self.assertEqual(output["without_gs_relays"][(2, 7)], (3, 0, 1))
        self.assertEqual(output["without_gs_relays"][(2, 8)], (8, 5, 0))
        self.assertEqual(output["without_gs_relays"][(2, 9)], (9, 6, 0))
        self.assertEqual(output["without_gs_relays"][(3, 5)], (2, 1, 0))
        self.assertEqual(output["without_gs_relays"][(3, 6)], (2, 1, 0))
        self.assertEqual(output["without_gs_relays"][(3, 7)], (0, 0, 0))
        self.assertEqual(output["without_gs_relays"][(3, 8)], (2, 1, 0))
        self.assertEqual(output["without_gs_relays"][(3, 9)], (2, 1, 0))
        self.assertEqual(output["without_gs_relays"][(4, 5)], (2, 1, 1))
        self.assertEqual(output["without_gs_relays"][(4, 6)], (1, 0, 0))
        self.assertEqual(output["without_gs_relays"][(4, 7)], (2, 1, 1))
        self.assertEqual(output["without_gs_relays"][(4, 8)], (2, 1, 1))
        self.assertEqual(output["without_gs_relays"][(4, 9)], (1, 0, 0))
        self.assertEqual(output["without_gs_relays"][(5, 6)], (2, 0, 2))
        self.assertEqual(output["without_gs_relays"][(5, 7)], (2, 0, 2))
        self.assertEqual(output["without_gs_relays"][(5, 8)], (2, 0, 2))
        self.assertEqual(output["without_gs_relays"][(5, 9)], (2, 0, 2))
        self.assertEqual(output["without_gs_relays"][(6, 5)], (1, 0, 2))
        self.assertEqual(output["without_gs_relays"][(6, 7)], (1, 0, 2))
        self.assertEqual(output["without_gs_relays"][(6, 8)], (1, 0, 2))
        self.assertEqual(output["without_gs_relays"][(6, 9)], (1, 0, 2))
        self.assertEqual(output["without_gs_relays"][(7, 5)], (0, 0, 3))
        self.assertEqual(output["without_gs_relays"][(7, 6)], (0, 0, 3))
        self.assertEqual(output["without_gs_relays"][(7, 8)], (0, 0, 3))
        self.assertEqual(output["without_gs_relays"][(7, 9)], (0, 0, 3))
        self.assertEqual(output["without_gs_relays"][(8, 5)], (2, 0, 5))
        self.assertEqual(output["without_gs_relays"][(8, 6)], (2, 0, 5))
        self.assertEqual(output["without_gs_relays"][(8, 7)], (0, 0, 4))
        self.assertEqual(output["without_gs_relays"][(8, 9)], (2, 0, 5))
        self.assertEqual(output["without_gs_relays"][(9, 5)], (2, 0, 6))
        self.assertEqual(output["without_gs_relays"][(9, 6)], (1, 0, 5))
        self.assertEqual(output["without_gs_relays"][(9, 7)], (2, 0, 6))
        self.assertEqual(output["without_gs_relays"][(9, 8)], (2, 0, 6))

        #
        #  7 --  0       1 -- 6
        #      / |       | \
        #    8    3     4   9
        #    |      \  /    |
        #    +------ 2 -----+
        #            |
        #            5
        #

        self.assertEqual(output["only_gs_relays"][(0, 5)], (8, 3, 0))
        self.assertEqual(output["only_gs_relays"][(0, 6)], (8, 3, 0))
        self.assertEqual(output["only_gs_relays"][(0, 7)], (7, 2, 0))
        self.assertEqual(output["only_gs_relays"][(0, 8)], (8, 3, 0))
        self.assertEqual(output["only_gs_relays"][(0, 9)], (8, 3, 0))
        self.assertEqual(output["only_gs_relays"][(1, 5)], (9, 4, 0))
        self.assertEqual(output["only_gs_relays"][(1, 6)], (6, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 7)], (9, 4, 0))
        self.assertEqual(output["only_gs_relays"][(1, 8)], (9, 4, 0))
        self.assertEqual(output["only_gs_relays"][(1, 9)], (9, 4, 0))
        self.assertEqual(output["only_gs_relays"][(2, 5)], (5, 0, 0))
        self.assertEqual(output["only_gs_relays"][(2, 6)], (9, 4, 0))
        self.assertEqual(output["only_gs_relays"][(2, 7)], (8, 3, 0))
        self.assertEqual(output["only_gs_relays"][(2, 8)], (8, 3, 0))
        self.assertEqual(output["only_gs_relays"][(2, 9)], (9, 4, 0))
        self.assertEqual(output["only_gs_relays"][(3, 5)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(3, 6)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(3, 7)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(3, 8)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(3, 9)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(4, 5)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(4, 6)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(4, 7)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(4, 8)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(4, 9)], (-1, -1, -1))
        self.assertEqual(output["only_gs_relays"][(5, 6)], (2, 0, 0))
        self.assertEqual(output["only_gs_relays"][(5, 7)], (2, 0, 0))
        self.assertEqual(output["only_gs_relays"][(5, 8)], (2, 0, 0))
        self.assertEqual(output["only_gs_relays"][(5, 9)], (2, 0, 0))
        self.assertEqual(output["only_gs_relays"][(6, 5)], (1, 0, 1))
        self.assertEqual(output["only_gs_relays"][(6, 7)], (1, 0, 1))
        self.assertEqual(output["only_gs_relays"][(6, 8)], (1, 0, 1))
        self.assertEqual(output["only_gs_relays"][(6, 9)], (1, 0, 1))
        self.assertEqual(output["only_gs_relays"][(7, 5)], (0, 0, 2))
        self.assertEqual(output["only_gs_relays"][(7, 6)], (0, 0, 2))
        self.assertEqual(output["only_gs_relays"][(7, 8)], (0, 0, 2))
        self.assertEqual(output["only_gs_relays"][(7, 9)], (0, 0, 2))
        self.assertEqual(output["only_gs_relays"][(8, 5)], (2, 0, 3))
        self.assertEqual(output["only_gs_relays"][(8, 6)], (2, 0, 3))
        self.assertEqual(output["only_gs_relays"][(8, 7)], (0, 0, 3))
        self.assertEqual(output["only_gs_relays"][(8, 9)], (2, 0, 3))
        self.assertEqual(output["only_gs_relays"][(9, 5)], (2, 0, 4))
        self.assertEqual(output["only_gs_relays"][(9, 6)], (1, 0, 4))
        self.assertEqual(output["only_gs_relays"][(9, 7)], (2, 0, 4))
        self.assertEqual(output["only_gs_relays"][(9, 8)], (2, 0, 4))

        #
        #  7 --  0       1 -- 6
        #      / |       | \
        #    8    3     4   9
        #    |      \  /    |
        #    +------ 2 -----+
        #            |
        #            5
        #

        self.assertEqual(output["combined"][(0, 5)], (8, 4, 0))
        self.assertEqual(output["combined"][(0, 6)], (8, 4, 0))
        self.assertEqual(output["combined"][(0, 7)], (7, 3, 0))
        self.assertEqual(output["combined"][(0, 8)], (8, 4, 0))
        self.assertEqual(output["combined"][(0, 9)], (8, 4, 0))
        self.assertEqual(output["combined"][(1, 5)], (4, 0, 0))
        self.assertEqual(output["combined"][(1, 6)], (6, 2, 0))
        self.assertEqual(output["combined"][(1, 7)], (4, 0, 0))
        self.assertEqual(output["combined"][(1, 8)], (4, 0, 0))
        self.assertEqual(output["combined"][(1, 9)], (9, 5, 0))
        self.assertEqual(output["combined"][(2, 5)], (5, 2, 0))
        self.assertEqual(output["combined"][(2, 6)], (4, 1, 1))
        self.assertEqual(output["combined"][(2, 7)], (8, 5, 0))
        self.assertEqual(output["combined"][(2, 8)], (8, 5, 0))
        self.assertEqual(output["combined"][(2, 9)], (9, 6, 0))
        self.assertEqual(output["combined"][(3, 5)], (2, 1, 0))
        self.assertEqual(output["combined"][(3, 6)], (2, 1, 0))
        self.assertEqual(output["combined"][(3, 7)], (0, 0, 0))
        self.assertEqual(output["combined"][(3, 8)], (2, 1, 0))
        self.assertEqual(output["combined"][(3, 9)], (2, 1, 0))
        self.assertEqual(output["combined"][(4, 5)], (2, 1, 1))
        self.assertEqual(output["combined"][(4, 6)], (1, 0, 0))
        self.assertEqual(output["combined"][(4, 7)], (2, 1, 1))
        self.assertEqual(output["combined"][(4, 8)], (2, 1, 1))
        self.assertEqual(output["combined"][(4, 9)], (1, 0, 0))
        self.assertEqual(output["combined"][(5, 6)], (2, 0, 2))
        self.assertEqual(output["combined"][(5, 7)], (2, 0, 2))
        self.assertEqual(output["combined"][(5, 8)], (2, 0, 2))
        self.assertEqual(output["combined"][(5, 9)], (2, 0, 2))
        self.assertEqual(output["combined"][(6, 5)], (1, 0, 2))
        self.assertEqual(output["combined"][(6, 7)], (1, 0, 2))
        self.assertEqual(output["combined"][(6, 8)], (1, 0, 2))
        self.assertEqual(output["combined"][(6, 9)], (1, 0, 2))
        self.assertEqual(output["combined"][(7, 5)], (0, 0, 3))
        self.assertEqual(output["combined"][(7, 6)], (0, 0, 3))
        self.assertEqual(output["combined"][(7, 8)], (0, 0, 3))
        self.assertEqual(output["combined"][(7, 9)], (0, 0, 3))
        self.assertEqual(output["combined"][(8, 5)], (2, 0, 5))
        self.assertEqual(output["combined"][(8, 6)], (2, 0, 5))
        self.assertEqual(output["combined"][(8, 7)], (0, 0, 4))
        self.assertEqual(output["combined"][(8, 9)], (2, 0, 5))
        self.assertEqual(output["combined"][(9, 5)], (2, 0, 6))
        self.assertEqual(output["combined"][(9, 6)], (1, 0, 5))
        self.assertEqual(output["combined"][(9, 7)], (2, 0, 6))
        self.assertEqual(output["combined"][(9, 8)], (2, 0, 6))

    def test_two_sat_two_gs_no_isl(self):

        #
        #     0     1
        #    /  \ /  \
        #  2     3    4
        #

        num_satellites = 2
        num_ground_stations = 3

        edges = [
            (0, 2, 100),
            (0, 3, 100),
            (1, 3, 100),
            (1, 4, 100),
        ]

        output = calculate_fstate_for(num_satellites, num_ground_stations, edges)

        self.assertEqual(output["without_gs_relays"][(0, 2)], (2, 0, 0))
        self.assertEqual(output["without_gs_relays"][(0, 3)], (3, 1, 0))
        self.assertEqual(output["without_gs_relays"][(0, 4)], (-1, -1, -1))
        self.assertEqual(output["without_gs_relays"][(1, 2)], (-1, -1, -1))
        self.assertEqual(output["without_gs_relays"][(1, 3)], (3, 1, 0))
        self.assertEqual(output["without_gs_relays"][(1, 4)], (4, 2, 0))
        self.assertEqual(output["without_gs_relays"][(2, 3)], (0, 0, 0))
        self.assertEqual(output["without_gs_relays"][(2, 4)], (-1, -1, -1))
        self.assertEqual(output["without_gs_relays"][(3, 2)], (0, 0, 1))
        self.assertEqual(output["without_gs_relays"][(3, 4)], (1, 0, 1))
        self.assertEqual(output["without_gs_relays"][(4, 2)], (-1, -1, -1))
        self.assertEqual(output["without_gs_relays"][(4, 3)], (1, 0, 2))

        self.assertEqual(output["only_gs_relays"][(0, 2)], (2, 0, 0))
        self.assertEqual(output["only_gs_relays"][(0, 3)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(0, 4)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 2)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 3)], (3, 1, 0))
        self.assertEqual(output["only_gs_relays"][(1, 4)], (4, 2, 0))
        self.assertEqual(output["only_gs_relays"][(2, 3)], (0, 0, 0))
        self.assertEqual(output["only_gs_relays"][(2, 4)], (0, 0, 0))
        self.assertEqual(output["only_gs_relays"][(3, 2)], (0, 0, 1))
        self.assertEqual(output["only_gs_relays"][(3, 4)], (1, 0, 1))
        self.assertEqual(output["only_gs_relays"][(4, 2)], (1, 0, 2))
        self.assertEqual(output["only_gs_relays"][(4, 3)], (1, 0, 2))

        self.assertEqual(output["combined"][(0, 2)], (2, 0, 0))
        self.assertEqual(output["combined"][(0, 3)], (3, 1, 0))
        self.assertEqual(output["combined"][(0, 4)], (3, 1, 0))
        self.assertEqual(output["combined"][(1, 2)], (3, 1, 0))
        self.assertEqual(output["combined"][(1, 3)], (3, 1, 0))
        self.assertEqual(output["combined"][(1, 4)], (4, 2, 0))
        self.assertEqual(output["combined"][(2, 3)], (0, 0, 0))
        self.assertEqual(output["combined"][(2, 4)], (0, 0, 0))
        self.assertEqual(output["combined"][(3, 2)], (0, 0, 1))
        self.assertEqual(output["combined"][(3, 4)], (1, 0, 1))
        self.assertEqual(output["combined"][(4, 2)], (1, 0, 2))
        self.assertEqual(output["combined"][(4, 3)], (1, 0, 2))
