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
import networkload
import random

local_shell = exputil.LocalShell()

# Clean-up for new a fresh run
local_shell.remove_force_recursive("runs")
local_shell.remove_force_recursive("pdf")
local_shell.remove_force_recursive("data")

for traffic_mode in ["specific", "general"]:
    for movement in ["static", "moving"]:

        # Prepare run directory
        run_dir = "runs/run_" + traffic_mode + "_tm_pairing_kuiper_isls_" + movement
        local_shell.remove_force_recursive(run_dir)
        local_shell.make_full_dir(run_dir)

        # config_ns3.properties
        local_shell.copy_file("templates/template_config_ns3.properties", run_dir + "/config_ns3.properties")
        local_shell.sed_replace_in_file_plain(
            run_dir + "/config_ns3.properties",
            "[SATELLITE-NETWORK-FORCE-STATIC]",
            "true" if movement == "static" else "false"
        )

        # Make logs_ns3 already for console.txt mapping
        local_shell.make_full_dir(run_dir + "/logs_ns3")

        # .gitignore (legacy reasons)
        local_shell.write_file(run_dir + "/.gitignore", "logs_ns3")

        # Traffic selection
        if traffic_mode == "specific":

            # Create the initial random reciprocal pairing with already one pair known (1174, 1229)
            random.seed(123456789)
            random.randint(0, 100000000)  # Legacy reasons
            seed_from_to = random.randint(0, 100000000)
            a = set(range(1156, 1256))
            a.remove(1174)
            a.remove(1229)
            initial_list_from_to = [(1174, 1229), (1229, 1174)]
            initial_list_from_to = initial_list_from_to + networkload.generate_from_to_reciprocated_random_pairing(
                list(a),
                seed_from_to
            )

            # Find all source and destination satellites of 1174 and 1229 by going over all its paths
            satellite_conflicts_set = set()
            with open(
                "../../satgenpy_analysis/data/"
                "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/100ms_for_200s/"
                "manual/data/networkx_path_1174_to_1229.txt", "r"
            ) as f_in:

                # Every reachable path, add to the set the source and destination
                # satellite (index 1, and second to last)
                for line in f_in:
                    line = line.strip()
                    path_str = line.split(",")[1]
                    if path_str != "Unreachable":
                        path = path_str.split("-")
                        satellite_conflicts_set.add(int(path[1]))
                        satellite_conflicts_set.add(int(path[-2]))
            satellite_conflicts = list(satellite_conflicts_set)

            # Now we need to remove the pairs which are sharing at any
            # point the source / destination satellite of a path between 1174 and 1229
            conflicting_pairs = []
            non_conflicting_pairs = [(1174, 1229), (1229, 1174)]
            local_shell.make_full_dir("extra_satgenpy_analysis_data")
            for p in initial_list_from_to[2:]:  # Of course excluding the starting (1174, 1229) and (1229, 1174) pairs

                # Resulting path filename
                resulting_path_filename = (
                        "extra_satgenpy_analysis_data/"
                        "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/"
                        "100ms_for_200s/manual/data/networkx_path_" + str(p[0]) + "_to_" + str(p[1]) + ".txt"
                )

                # Generate the path file if it does not exist yet (expensive)
                if not local_shell.file_exists(resulting_path_filename):
                    local_shell.perfect_exec(
                        "cd ../../../satgenpy; python -m satgen.post_analysis.main_print_routes_and_rtt "
                        "../paper/ns3_experiments/traffic_matrix/extra_satgenpy_analysis_data "
                        "../paper/satellite_networks_state/gen_data/"
                        "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls "
                        "100 200 " + str(p[0]) + " " + str(p[1])
                    )

                # Open the path file
                with open(resulting_path_filename, "r") as f_in:

                    # If any of its paths have a source or destination satellite which is one of the conflicts,
                    # then it is a conflict and the pair is not added
                    conflict = False
                    for line in f_in:
                        path_str = line.split(",")[1]
                        if path_str != "Unreachable":
                            path = path_str.split("-")
                            if int(path[1]) in satellite_conflicts or int(path[-2]) in satellite_conflicts:
                                conflict = True
                    if conflict:
                        conflicting_pairs.append(p)
                    else:
                        non_conflicting_pairs.append(p)

            # Final prints
            # print("Original pairs (%d):" % len(initial_list_from_to))
            # print(initial_list_from_to)
            # print("")
            # print("Conflicting pairs (%d/%d):" % (len(conflicting_pairs), len(initial_list_from_to)))
            # print(conflicting_pairs)
            # print("")
            # print("Final non-conflicting pairs (%d/%d):" % (len(non_conflicting_pairs), len(initial_list_from_to)))
            # print(non_conflicting_pairs)
            # print("")

            # Check it matches the legacy expectation
            if non_conflicting_pairs != [
                (1174, 1229), (1229, 1174), (1214, 1166), (1166, 1214), (1205, 1251), (1251, 1205), (1165, 1213),
                (1213, 1165), (1244, 1196), (1196, 1244), (1157, 1253), (1253, 1157), (1220, 1167), (1167, 1220),
                (1212, 1197), (1197, 1212), (1178, 1217), (1217, 1178), (1250, 1199), (1199, 1250), (1202, 1163),
                (1163, 1202), (1247, 1198), (1198, 1247), (1238, 1187), (1187, 1238), (1239, 1164), (1164, 1239),
                (1241, 1181), (1181, 1241), (1248, 1184), (1184, 1248), (1173, 1221), (1221, 1173), (1195, 1254),
                (1254, 1195), (1193, 1243), (1243, 1193), (1249, 1185), (1185, 1249), (1207, 1162), (1162, 1207),
                (1226, 1209), (1209, 1226), (1227, 1176), (1176, 1227), (1210, 1245), (1245, 1210), (1188, 1200),
                (1200, 1188), (1233, 1231), (1231, 1233), (1208, 1255), (1255, 1208), (1204, 1189), (1189, 1204),
                (1201, 1228), (1228, 1201), (1206, 1186), (1186, 1206), (1169, 1237), (1237, 1169), (1222, 1194),
                (1194, 1222), (1223, 1218), (1218, 1223), (1190, 1211), (1211, 1190), (1236, 1158), (1158, 1236),
                (1182, 1203), (1203, 1182), (1172, 1235), (1235, 1172), (1242, 1224), (1224, 1242), (1191, 1216),
                (1216, 1191), (1171, 1168), (1168, 1171), (1240, 1170), (1170, 1240), (1230, 1219), (1219, 1230),
                (1192, 1160), (1160, 1192), (1161, 1232), (1232, 1161)
            ]:
                raise ValueError("Final generated non-conflicting pairs is not what was expected")

            # Final from-to list
            list_from_to = non_conflicting_pairs

        elif traffic_mode == "general":

            # Create a random reciprocal pairing with already one pair known (1174, 1229)
            random.seed(123456789)
            random.randint(0, 100000000)  # Legacy reasons
            seed_from_to = random.randint(0, 100000000)
            a = set(range(1156, 1256))
            a.remove(1174)
            a.remove(1229)
            list_from_to = [(1174, 1229), (1229, 1174)]
            list_from_to = list_from_to + networkload.generate_from_to_reciprocated_random_pairing(
                list(a),
                seed_from_to
            )

        else:
            raise ValueError("Unknown traffic mode: " + traffic_mode)

        # Write the schedule
        networkload.write_schedule(
            run_dir + "/schedule_kuiper_630.csv",
            len(list_from_to),
            list_from_to,
            [1000000000000] * len(list_from_to),
            [0] * len(list_from_to)
        )

# Finished successfully
print("Success")
