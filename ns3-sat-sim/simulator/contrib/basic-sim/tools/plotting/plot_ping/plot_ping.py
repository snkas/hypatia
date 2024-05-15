import sys
from exputil import *


def plot_ping(logs_ns3_dir, data_out_dir, pdf_out_dir, from_id, to_id, interval_ns):
    local_shell = LocalShell()

    # Check that all plotting files are available
    if not local_shell.file_exists("plot_ping_time_vs_rtt.plt") or \
            not local_shell.file_exists("plot_ping_time_vs_out_of_order.plt"):
        print("The gnuplot files are not present.")
        print("Are you executing this python file inside the plot_ping directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)

    # Read in pingmesh CSV
    pingmesh_csv_columns = read_csv_direct_in_columns(
        logs_ns3_dir + "/pingmesh.csv",
        "pos_int,pos_int,pos_int,pos_int,int,int,int,int,int,string"
    )
    num_entries = len(pingmesh_csv_columns[0])
    from_list = pingmesh_csv_columns[0]
    to_list = pingmesh_csv_columns[1]
    ping_no_list = pingmesh_csv_columns[2]
    send_request_timestamp_list = pingmesh_csv_columns[3]
    reply_timestamp_list = pingmesh_csv_columns[4]
    receive_reply_timestamp_list = pingmesh_csv_columns[5]
    latency_to_there_ns_list = pingmesh_csv_columns[6]
    latency_from_there_ns_list = pingmesh_csv_columns[7]
    rtt_ns_list = pingmesh_csv_columns[8]
    arrived_list = pingmesh_csv_columns[9]

    # Find all entries for our pair
    ping_results = []
    prev_ping_no = -1
    for i in range(num_entries):
        if from_list[i] == from_id and to_list[i] == to_id:

            # Some checks
            if ping_no_list[i] != prev_ping_no + 1:
                raise ValueError("Ping number must be incrementally ascending")
            prev_ping_no += 1
            if arrived_list[i] == "YES":
                is_lost = False
            elif arrived_list[i] == "LOST":
                is_lost = True
            else:
                raise ValueError("Invalid value for arrival: " + arrived_list[i])

            # If lost
            if is_lost:
                if reply_timestamp_list[i] != -1 \
                        or receive_reply_timestamp_list[i] != -1 \
                        or latency_to_there_ns_list[i] != -1 \
                        or latency_from_there_ns_list[i] != -1 \
                        or rtt_ns_list[i] != -1:
                    raise ValueError("Is lost so invalid timestamps should all be -1")
            else:
                parse_positive_int(str(reply_timestamp_list[i]))
                parse_positive_int(str(receive_reply_timestamp_list[i]))
                parse_positive_int(str(latency_to_there_ns_list[i]))
                parse_positive_int(str(latency_from_there_ns_list[i]))
                parse_positive_int(str(rtt_ns_list[i]))

            # Add to ping results found
            ping_results.append({
                "ping_no": ping_no_list[i],
                "send_request_timestamp": send_request_timestamp_list[i],
                "reply_timestamp": reply_timestamp_list[i],
                "receive_reply_timestamp": receive_reply_timestamp_list[i],
                "latency_to_there_ns": latency_to_there_ns_list[i],
                "latency_from_there_ns": latency_from_there_ns_list[i],
                "rtt_ns": rtt_ns_list[i],
                "is_lost": is_lost
            })

    # Data RTT file
    print("TIME VS. RTT")
    data_rtt_filename = "%s/ping_%d_to_%d_rtt.csv" % (data_out_dir, from_id, to_id)
    with open(data_rtt_filename, "w+") as out_file:
        for val in ping_results:
            if val["is_lost"]:
                out_file.write("%d,%d,%d,0\n" % (from_id, to_id, val["send_request_timestamp"]))
            else:
                out_file.write("%d,%d,%d,%d\n" % (from_id, to_id, val["send_request_timestamp"], val["rtt_ns"]))

    # Show what is produced
    print(" > Data line format..... [from_id],[to_id],[send_request_timestamp (ns)],[measured_rtt (ns); 0 if lost]")
    print(" > Produced data file... " + data_rtt_filename)

    # Plot time vs. rtt
    pdf_rtt_filename = "%s/plot_ping_time_vs_rtt_%d_to_%d.pdf" % (pdf_out_dir, from_id, to_id)
    local_shell.copy_file("plot_ping_time_vs_rtt.plt", "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_rtt_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_rtt_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    local_shell.remove("temp.plt")
    print(" > Produced plot........ " + pdf_rtt_filename)
    print("")

    # Data out-of-order file
    print("TIME VS. OUT-OF-ORDER")

    # For all intervals determine the number of out-of-order
    current_interval = (0, interval_ns, 0)
    intervals = []

    # Now we need to check which ones are out of order
    # (a) It is marked as lost
    # (b) Its reply arrives after that of the reply of one of the next pings
    current_lowest_timestamp = 1000000000000000
    is_out_of_order = [False] * len(ping_results)
    for i in reversed(range(len(ping_results))):
        entry = ping_results[i]
        if entry["is_lost"]:
            is_out_of_order[i] = True
        else:
            if entry["receive_reply_timestamp"] > current_lowest_timestamp:
                is_out_of_order[i] = True
            current_lowest_timestamp = min(current_lowest_timestamp, entry["receive_reply_timestamp"])

    # Go forward to fill up the interval counts
    for i in range(len(ping_results)):
        entry = ping_results[i]

        # Continue to fast-forward intervals until the next entry is in it
        while entry["send_request_timestamp"] >= current_interval[1]:
            intervals.append(current_interval)
            current_interval = (current_interval[1], current_interval[1] + interval_ns, 0)

        # Now it must be within current_interval
        if is_out_of_order[i]:
            current_interval = (
                current_interval[0],
                current_interval[1],
                current_interval[2] + 1
            )

    # Add the last interval always
    intervals.append(current_interval)

    # Each interval [a, b] with progress c, gets converted into two points:
    # a, c
    # b - (small number), c
    #
    # This effectively creates a step function as a continuous line, which can then be plotted by gnuplot.
    #
    data_out_of_order_filename = "%s/ping_%d_to_%d_out_of_order_in_intervals.csv" % (data_out_dir, from_id, to_id)
    with open(data_out_of_order_filename, "w+") as f_out:
        for i in range(len(intervals)):
            f_out.write("%d,%d,%.10f,%.10f\n" % (from_id, to_id, intervals[i][0], intervals[i][2]))
            f_out.write("%d,%d,%.10f,%.10f\n" % (from_id, to_id, intervals[i][1] - 0.000001, intervals[i][2]))
    print(" > Calculated in intervals of " + str(interval_ns / 1e6) + " ms")
    print(" > Data line format..... [from_id],[to_id],[time moment (ns)],[# of out-of-order]")
    print(" > Produced data file... " + data_out_of_order_filename)

    # Plot time vs. out-of-order
    pdf_out_of_order_filename = "%s/plot_ping_time_vs_out_of_order_%d_to_%d.pdf" % (pdf_out_dir, from_id, to_id)
    local_shell.copy_file("plot_ping_time_vs_out_of_order.plt", "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_out_of_order_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_out_of_order_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    local_shell.remove("temp.plt")
    print(" > Produced plot........ " + pdf_out_of_order_filename)
    print("")


def main():
    args = sys.argv[1:]
    if len(args) != 6:
        print("Must supply exactly six arguments")
        print("Usage: python plot_ping.py [logs_ns3 directory] [data_out_dir] [pdf_out_dir] "
              "[from_node_id] [to_node_id] [interval_ns (for out-of-order counting)]")
        exit(1)
    else:
        plot_ping(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4]),
            int(args[5])
        )


if __name__ == "__main__":
    main()
