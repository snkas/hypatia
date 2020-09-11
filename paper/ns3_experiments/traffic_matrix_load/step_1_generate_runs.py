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

local_shell.remove_force_recursive("runs")
local_shell.remove_force_recursive("pdf")
local_shell.remove_force_recursive("data")

for config in [
    # Rate in Mbit/s, duration in seconds, ISL network device queue size pkt for TCP, GSL network device queue size pkt for TCP
    # (UDP queue size is capped at 100)
    (1.0, 10, 10, 10),
    (1.0, 20, 10, 10),
    (1.0, 50, 10, 10),
    (10.0, 10, 100, 100),
    (10.0, 20, 100, 100),
    (10.0, 50, 100, 100),
    (25.0, 10, 250, 250),
    (25.0, 20, 250, 250),
    (25.0, 50, 250, 250),
    (100.0, 10, 1000, 1000),
    (100.0, 20, 1000, 1000),
    (250.0, 10, 2500, 2500),
    (250.0, 20, 2500, 2500),
    (1000.0, 10, 10000, 10000),
    (2500.0, 10, 25000, 25000),
    (10000.0, 1, 100000, 100000),
    (10000.0, 2, 100000, 100000),
]:

    # Retrieve values from the config
    data_rate_megabit_per_s = config[0]
    duration_s = config[1]

    # Both protocols
    for protocol_chosen in ["tcp", "udp"]:

        # TCP NewReno needs at least the BDP in queue size to fulfill bandwidth
        if protocol_chosen == "tcp":
            queue_size_isl_pkt = config[2]
            queue_size_gsl_pkt = config[3]
        elif protocol_chosen == "udp":  # UDP does not have this problem, so we cap it at 100 packets
            queue_size_isl_pkt = min(config[2], 100)
            queue_size_gsl_pkt = min(config[3], 100)
        else:
            raise ValueError("Unknown protocol chosen: " + protocol_chosen)

        # Prepare run directory
        run_dir = "runs/run_loaded_tm_pairing_%d_Mbps_for_%ds_with_%s" % (
            data_rate_megabit_per_s, duration_s, protocol_chosen
        )
        local_shell.remove_force_recursive(run_dir)
        local_shell.make_full_dir(run_dir)

        # config_ns3.properties
        local_shell.copy_file(
            "templates/template_config_ns3_" + protocol_chosen + ".properties",
            run_dir + "/config_ns3.properties"
        )
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[SIMULATION-END-TIME-NS]", str(duration_s * 1000 * 1000 * 1000))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[ISL-DATA-RATE-MEGABIT-PER-S]", str(data_rate_megabit_per_s))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[GSL-DATA-RATE-MEGABIT-PER-S]", str(data_rate_megabit_per_s))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[ISL-MAX-QUEUE-SIZE-PKTS]", str(queue_size_isl_pkt))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[GSL-MAX-QUEUE-SIZE-PKTS]", str(queue_size_gsl_pkt))

        # Make logs_ns3 already for console.txt mapping
        local_shell.make_full_dir(run_dir + "/logs_ns3")

        # .gitignore (legacy reasons)
        local_shell.write_file(run_dir + "/.gitignore", "logs_ns3")

        # Schedule
        random.seed(123456789)
        random.randint(0, 100000000)  # Legacy reasons
        seed_from_to = random.randint(0, 100000000)
        a = set(range(1156, 1256))
        list_from_to = networkload.generate_from_to_reciprocated_random_pairing(list(a), seed_from_to)

        # tcp_flow_schedule.csv
        if protocol_chosen == "tcp":
            networkload.write_schedule(
                run_dir + "/tcp_flow_schedule.csv",
                len(list_from_to),
                list_from_to,
                [1000000000000] * len(list_from_to),
                [0] * len(list_from_to)
            )

        # udp_burst_schedule.csv
        elif protocol_chosen == "udp":
            with open(run_dir + "/udp_burst_schedule.csv", "w+") as f_out:
                for i in range(len(list_from_to)):
                    f_out.write(
                        "%d,%d,%d,%.10f,%d,%d,,\n" % (
                            i,
                            list_from_to[i][0],
                            list_from_to[i][1],
                            data_rate_megabit_per_s,
                            0,
                            1000000000000
                        )
                    )
