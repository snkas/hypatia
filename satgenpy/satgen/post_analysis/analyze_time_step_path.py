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
from satgen.ground_stations import *
from satgen.tles import *
import exputil
from statsmodels.distributions.empirical_distribution import ECDF


def analyze_time_step_path(output_data_dir, satellite_network_dir,
                           multiple_dynamic_state_update_interval_ms, simulation_end_time_s):

    # Variables (load in for each thread such that they don't interfere)
    ground_stations = read_ground_stations_extended(satellite_network_dir + "/ground_stations.txt")
    tles = read_tles(satellite_network_dir + "/tles.txt")
    satellites = tles["satellites"]

    # Local shell
    local_shell = exputil.LocalShell()
    core_network_folder_name = satellite_network_dir.split("/")[-1]
    base_output_dir = "%s/%s/%ds/path" % (
        output_data_dir, core_network_folder_name, simulation_end_time_s
    )
    pdf_dir = base_output_dir + "/pdf"
    data_dir = base_output_dir + "/data"
    local_shell.remove_force_recursive(pdf_dir)
    local_shell.remove_force_recursive(data_dir)
    local_shell.make_full_dir(pdf_dir)
    local_shell.make_full_dir(data_dir)

    # Configs
    configs = []
    for i in range(len(multiple_dynamic_state_update_interval_ms)):
        configs.append(
            (
                multiple_dynamic_state_update_interval_ms[i],
                "%s/dynamic_state_%dms_for_%ds" % (
                    satellite_network_dir, multiple_dynamic_state_update_interval_ms[i], simulation_end_time_s
                ),
            )
        )

    # Derivatives
    simulation_end_time_ns = simulation_end_time_s * 1000 * 1000 * 1000

    # Analysis
    per_dyn_state_path_list_per_pair = []
    for c in range(len(configs)):
        path_list_per_pair = []
        for i in range(len(ground_stations)):
            temp_list = []
            for j in range(len(ground_stations)):
                temp_list.append([])
            path_list_per_pair.append(temp_list)
        per_dyn_state_path_list_per_pair.append(path_list_per_pair)

    # For each time moment
    fstate = {}
    smallest_step_ns = min(multiple_dynamic_state_update_interval_ms) * 1000 * 1000
    num_iterations = simulation_end_time_ns / smallest_step_ns
    it = 1
    for t in range(0, simulation_end_time_ns, smallest_step_ns):

        c_idx = 0
        for c in configs:
            if t % (c[0] * 1000 * 1000) == 0:

                # Read in forwarding state
                with open(c[1] + "/fstate_" + str(t) + ".txt", "r") as f_in:
                    for line in f_in:
                        spl = line.split(",")
                        current = int(spl[0])
                        destination = int(spl[1])
                        next_hop = int(spl[2])
                        fstate[(current, destination)] = next_hop

                    # Go over each pair of ground stations and calculate the length
                    for src in range(len(ground_stations)):
                        for dst in range(src + 1, len(ground_stations)):
                            src_node_id = len(satellites) + src
                            dst_node_id = len(satellites) + dst
                            path = get_path(src_node_id, dst_node_id, fstate)
                            path_list_per_pair = per_dyn_state_path_list_per_pair[c_idx]
                            if path is None:
                                if len(path_list_per_pair[src][dst]) == 0 or [] != path_list_per_pair[src][dst][-1][0]:
                                    path_list_per_pair[src][dst].append(([], t))

                            else:
                                if len(path_list_per_pair[src][dst]) == 0 \
                                        or path != path_list_per_pair[src][dst][-1][0]:
                                    path_list_per_pair[src][dst].append((path, t))

            c_idx += 1

        # Show progress a bit
        print("%d / %d" % (it, num_iterations))
        it += 1
    print("")

    # Calculate path overlap
    time_between_path_change_ns_list = []
    per_config_pair_missed_path_changes_list = []
    for c_idx in range(len(configs)):
        per_config_pair_missed_path_changes_list.append([])
    for src in range(len(ground_stations)):
        for dst in range(src + 1, len(ground_stations)):
            base_path_list = per_dyn_state_path_list_per_pair[0][src][dst]
            for j in range(2, len(base_path_list)):  # First change is from epoch, which is not representative
                time_between_path_change_ns_list.append(base_path_list[j][1] - base_path_list[j - 1][1])
            for c_idx in range(0, len(configs)):
                worse_path_list = per_dyn_state_path_list_per_pair[c_idx][src][dst]
                per_config_pair_missed_path_changes_list[c_idx].append(len(base_path_list) - len(worse_path_list))

    #################################################

    # Write and plot ECDFs
    for element in [
        ("ecdf_overall_time_between_path_change", ECDF(time_between_path_change_ns_list)),
    ]:
        name = element[0]
        ecdf = element[1]
        with open(data_dir + "/" + name + ".txt", "w+") as f_out:
            for i in range(len(ecdf.x)):
                f_out.write(str(ecdf.x[i]) + "," + str(ecdf.y[i]) + "\n")

    # Find all the lists
    for c_idx in range(len(configs)):
        config = configs[c_idx]

        # Write and plot ECDFs
        for element in [
            ("ecdf_pairs_%dms_missed_path_changes" % config[0], ECDF(per_config_pair_missed_path_changes_list[c_idx])),
        ]:
            name = element[0]
            ecdf = element[1]
            with open(data_dir + "/" + name + ".txt", "w+") as f_out:
                for i in range(len(ecdf.x)):
                    f_out.write(str(ecdf.x[i]) + "," + str(ecdf.y[i]) + "\n")

    # Histograms
    with open(data_dir + "/histogram_missed_path_changes.txt", "w+") as f_out:
        f_out.write("Granularity ")
        for x in range(100):
            f_out.write(" " + str(x))
        f_out.write("\n")
        for c_idx in range(1, len(configs)):
            config = configs[c_idx]
            f_out.write(str(config[0]) + "ms")
            counter = [0]*100
            for a in per_config_pair_missed_path_changes_list[c_idx]:
                counter[a] += 1
            for x in counter:
                f_out.write(" " + str(x / len(per_config_pair_missed_path_changes_list[c_idx])))
            f_out.write("\n")

    print("Done")
