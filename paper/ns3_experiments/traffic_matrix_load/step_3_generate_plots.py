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
import numpy as np

local_shell = exputil.LocalShell()

# Create the data files
local_shell.remove_force_recursive("data")
local_shell.make_full_dir("data")
with open("data/traffic_goodput_total_data_sent_vs_runtime.csv", "w+") \
     as f_out_data_sent_vs_runtime, \
     open("data/traffic_goodput_rate_vs_slowdown.csv", "w+") \
     as f_out_rate_vs_slowdown, \
     open("data/run_dirs.csv", "w+") \
     as f_out_run_dirs:

    for protocol_chosen in ["tcp", "udp"]:

        for run_dir_details in [
            ("runs/run_loaded_tm_pairing_1_Mbps_for_10s_with_" + protocol_chosen, 10.0),
            ("runs/run_loaded_tm_pairing_1_Mbps_for_20s_with_" + protocol_chosen, 20.0),
            ("runs/run_loaded_tm_pairing_1_Mbps_for_50s_with_" + protocol_chosen, 50.0),
            ("runs/run_loaded_tm_pairing_10_Mbps_for_10s_with_" + protocol_chosen, 10.0),
            ("runs/run_loaded_tm_pairing_10_Mbps_for_20s_with_" + protocol_chosen, 20.0),
            ("runs/run_loaded_tm_pairing_10_Mbps_for_50s_with_" + protocol_chosen, 50.0),
            ("runs/run_loaded_tm_pairing_25_Mbps_for_10s_with_" + protocol_chosen, 10.0),
            ("runs/run_loaded_tm_pairing_25_Mbps_for_20s_with_" + protocol_chosen, 20.0),
            ("runs/run_loaded_tm_pairing_25_Mbps_for_50s_with_" + protocol_chosen, 50.0),
            ("runs/run_loaded_tm_pairing_100_Mbps_for_10s_with_" + protocol_chosen, 10.0),
            ("runs/run_loaded_tm_pairing_100_Mbps_for_20s_with_" + protocol_chosen, 20.0),
            ("runs/run_loaded_tm_pairing_250_Mbps_for_10s_with_" + protocol_chosen, 10.0),
            ("runs/run_loaded_tm_pairing_250_Mbps_for_20s_with_" + protocol_chosen, 20.0),
            ("runs/run_loaded_tm_pairing_1000_Mbps_for_10s_with_" + protocol_chosen, 10.0),
            ("runs/run_loaded_tm_pairing_2500_Mbps_for_10s_with_" + protocol_chosen, 10.0),
            ("runs/run_loaded_tm_pairing_10000_Mbps_for_1s_with_" + protocol_chosen, 1.0),
            ("runs/run_loaded_tm_pairing_10000_Mbps_for_2s_with_" + protocol_chosen, 2.0),
        ]:

            # Determine run directory
            run_dir = run_dir_details[0]
            duration_s = run_dir_details[1]
            logs_ns3_dir = run_dir + "/logs_ns3"

            # Finished filename to check if done
            finished_filename = logs_ns3_dir + "/finished.txt"

            if not (exputil.LocalShell().file_exists(finished_filename)
                    and exputil.LocalShell().read_file(finished_filename).strip() == "Yes"):
                print("Skipping: " + run_dir)

            else:
                print("Processing: " + run_dir)

                if protocol_chosen == "tcp":

                    # Sum up all goodput
                    tcp_flows_csv_columns = exputil.read_csv_direct_in_columns(
                        logs_ns3_dir + "/tcp_flows.csv",
                        "idx_int,pos_int,pos_int,pos_int,pos_int,pos_int,pos_int,pos_int,string,string"
                    )
                    amount_sent_byte_list = tcp_flows_csv_columns[7]
                    total_sent_byte = float(np.sum(amount_sent_byte_list))

                elif protocol_chosen == "udp":

                    # Sum up all goodput
                    udp_bursts_incoming_csv_columns = exputil.read_csv_direct_in_columns(
                        logs_ns3_dir + "/udp_bursts_incoming.csv",
                        "idx_int,pos_int,pos_int,pos_float,pos_int,pos_int,pos_float,pos_float,pos_float,pos_float,pos_float,string"
                    )
                    amount_payload_sent_byte_list = udp_bursts_incoming_csv_columns[10]
                    total_sent_byte = float(np.sum(amount_payload_sent_byte_list))

                # Sum up total runtime
                timing_results_csv_columns = exputil.read_csv_direct_in_columns(
                    logs_ns3_dir + "/timing_results.csv",
                    "string,pos_int"
                )
                duration_ns_list = timing_results_csv_columns[1]
                total_duration_ns = float(np.sum(duration_ns_list))

                # Write into data files

                # tcp/udp,<total sent (byte),<duration (ns)>
                f_out_data_sent_vs_runtime.write("%s,%.10f,%.10f\n" % (
                    protocol_chosen,
                    total_sent_byte,
                    total_duration_ns
                ))

                # tcp/udp,<Mbit/s>,<slow-down (real s / sim s)
                f_out_rate_vs_slowdown.write("%s,%.10f,%.10f\n" % (
                    protocol_chosen,
                    (total_sent_byte / duration_s) / 125000.0,
                    (total_duration_ns / 1e9) / duration_s
                ))

                # Run directory (to investigate)
                f_out_run_dirs.write(run_dir + "\n")

# Execute plots
local_shell.remove_force_recursive("pdf")
local_shell.make_full_dir("pdf")
local_shell.perfect_exec("cd plots; gnuplot plot_goodput_total_data_sent_vs_runtime.plt")
local_shell.perfect_exec("cd plots; gnuplot plot_goodput_rate_vs_slowdown.plt")
