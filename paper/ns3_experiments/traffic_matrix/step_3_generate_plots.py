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


def plot_pair_path_max_utilization(path_networkx_data, run_name, src_node_id, dst_node_id, is_static):

    # Read in the paths (list of: (time, path as a node list))
    paths = []
    with open(path_networkx_data + "/networkx_path_%d_to_%d.txt" % (src_node_id, dst_node_id), "r") as f_path:
        for line in f_path:
            spl = line.split(",")
            if spl[1].strip() == "Unreachable":
                paths.append((int(spl[0]), []))
            else:
                paths.append((int(spl[0]), list(map(lambda x: int(x), spl[1].split("-")))))
            if is_static:
                break

    # Read in the utilization file
    link_to_utilization = {}
    with open("runs/" + run_name + "/logs_ns3/isl_utilization.csv", "r") as f_utilization_in:
        for line in f_utilization_in:
            spl = line.split(",")
            from_id = int(spl[0])
            to_id = int(spl[1])
            from_time_ns = int(spl[2])
            till_time_ns = int(spl[3])
            utilization = float(spl[4])
            if from_time_ns == 0:
                link_to_utilization[(from_id, to_id)] = []
            link_to_utilization[(from_id, to_id)].append((from_time_ns, till_time_ns, utilization))

    # Create data and pdf filenames
    exputil.LocalShell().make_full_dir("data/" + run_name)
    exputil.LocalShell().make_full_dir("pdf/" + run_name)

    # The three variables we keep track of
    number_of_intervals_total = 0
    number_of_intervals_with_a_path = 0
    number_of_intervals_with_at_least_a_third_unused_bandwidth = 0
    all_intervals = []
    data_filename_at_100ms = "data/%s/pair_path_utilization_at_100ms_%d_to_%d.txt" % (
        run_name, src_node_id, dst_node_id
    )
    with open(data_filename_at_100ms, "w+") as f_out:
        path_idx = -1
        for t in range(0, 200 * 1000 * 1000 * 1000, 100 * 1000 * 1000):

            # If the next path takes over
            if path_idx != len(paths) - 1 and paths[path_idx + 1][0] == t:
                path_idx += 1

            # Fetch the utilization of all the ISL links on the path
            utilization_list = []
            for i in range(2, len(paths[path_idx][1]) - 1):
                pair = paths[path_idx][1][i - 1], paths[path_idx][1][i]
                for (from_time_ns, till_time_ns, utilization) in link_to_utilization[pair]:
                    if from_time_ns <= t < till_time_ns:
                        utilization_list.append(utilization)

            # And finally write the result
            f_out.write(str(t) + "," + str(max(utilization_list) if len(utilization_list) > 0 else 0) + "\n")
            all_intervals.append((t, max(utilization_list) if len(utilization_list) > 0 else 0))

            # If there was a path, find what the max utilization was,
            # and then count if the utilization was less than 2/3rds
            if len(utilization_list) > 0:
                if max(utilization_list) < 2.0 / 3.0:
                    number_of_intervals_with_at_least_a_third_unused_bandwidth += 1
                number_of_intervals_with_a_path += 1
            number_of_intervals_total += 1

    # Print the statistics
    data_filename_utilization_information = "data/%s/utilization_information_%d_to_%d.txt" % (
        run_name, src_node_id, dst_node_id
    )
    with open(data_filename_utilization_information, "w+") as f_out:
        s = "Total intervals.............................. %d" % number_of_intervals_total
        f_out.write(s + "\n")
        print(s)

        s = "Intervals with a path........................ %d" % number_of_intervals_with_a_path
        f_out.write(s + "\n")
        print(s)

        s = "Intervals (w/path) with utilization <= 2/3... %d" % (
              number_of_intervals_with_at_least_a_third_unused_bandwidth
        )
        f_out.write(s + "\n")
        print(s)

        s = "%.2f%% of the intervals have at least 33%% unused bandwidth" % (
                float(number_of_intervals_with_at_least_a_third_unused_bandwidth)
                /
                float(number_of_intervals_with_a_path) * 100.0
        )
        f_out.write(s + "\n")
        print(s)

    # Write the pair path utilization at 1 second granularity
    data_filename_at_1s = "data/%s/pair_path_utilization_at_1s_%d_to_%d.txt" % (run_name, src_node_id, dst_node_id)
    with open(data_filename_at_1s, "w+") as f_out_1s:
        second_utilization_sum = 0.0
        for i in range(1, len(all_intervals) + 1):
            second_utilization_sum += all_intervals[i - 1][1]
            if i % 10 == 0:
                f_out_1s.write("%d,%.20f\n" % (
                    (i / 10 - 1) * 1000 * 1000 * 1000,
                    second_utilization_sum / 10.0
                ))
                second_utilization_sum = 0.0

    # Perform the final plot
    pdf_filename = "pdf/%s/pair_available_bandwidth_%d_to_%d.pdf" % (run_name, src_node_id, dst_node_id)
    local_shell = exputil.LocalShell()
    if is_static:
        local_shell.copy_file("plots/plot_pair_path_available_bandwidth_no_red_box.plt", "temp.plt")
    else:
        local_shell.copy_file("plots/plot_pair_path_available_bandwidth.plt", "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename_at_1s)
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    local_shell.remove("temp.plt")


def main():

    # Create the data file
    local_shell = exputil.LocalShell()
    local_shell.remove_force_recursive("data")
    local_shell.make_full_dir("data")
    local_shell.remove_force_recursive("pdf")
    local_shell.make_full_dir("pdf")

    # Plot all the pair path utilization
    for traffic_mode in ["specific", "general"]:
        for movement in ["static", "moving"]:

            # Pair path max utilization
            plot_pair_path_max_utilization(
                "../../satgenpy_analysis/data/"
                "kuiper_630_isls_plus_grid_ground_stations_top_100_algorithm_free_one_only_over_isls/100ms_for_200s"
                "/manual/data",
                "run_%s_tm_pairing_kuiper_isls_%s" % (traffic_mode, movement),
                1174, 1229, movement == "static"
            )

            # Perform simple flow plot for debugging purposes
            run_name = "run_%s_tm_pairing_kuiper_isls_%s" % (traffic_mode, movement)
            local_shell.make_full_dir("pdf/" + run_name)
            local_shell.make_full_dir("data/" + run_name)
            local_shell.perfect_exec(
                "cd ../../../ns3-sat-sim/simulator/contrib/basic-sim/tools/plotting/plot_tcp_flow; "
                "python plot_tcp_flow.py "
                "../../../../../../../paper/ns3_experiments/traffic_matrix/runs/" + run_name + "/logs_ns3 "
                "../../../../../../../paper/ns3_experiments/traffic_matrix/data/" + run_name + " "
                "../../../../../../../paper/ns3_experiments/traffic_matrix/pdf/" + run_name + " "
                + "0 " + str(1 * 1000 * 1000 * 1000),  # Flow id = 0, 1 * 1000 * 1000 * 1000 ns = 1s interval
                output_redirect=exputil.OutputRedirect.CONSOLE
            )

            local_shell.perfect_exec(
                "cd ../../../ns3-sat-sim/simulator/contrib/basic-sim/tools/plotting/plot_tcp_flow; "
                "python plot_tcp_flow.py "
                "../../../../../../../paper/ns3_experiments/traffic_matrix/runs/" + run_name + "/logs_ns3 "
                "../../../../../../../paper/ns3_experiments/traffic_matrix/data/" + run_name + " "
                "../../../../../../../paper/ns3_experiments/traffic_matrix/pdf/" + run_name + " "
                + "35 " + str(1 * 1000 * 1000 * 1000),  # Flow id = 35, 1 * 1000 * 1000 * 1000 ns = 1s interval
                output_redirect=exputil.OutputRedirect.CONSOLE
            )


if __name__ == "__main__":
    main()
