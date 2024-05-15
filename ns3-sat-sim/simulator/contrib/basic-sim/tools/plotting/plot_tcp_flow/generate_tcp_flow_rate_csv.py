import exputil
import sys


def generate_tcp_flow_rate_csv(logs_ns3_dir, data_out_dir, tcp_flow_id, interval_ns):

    # Read in CSV of the progress
    progress_csv_columns = exputil.read_csv_direct_in_columns(
        logs_ns3_dir + "/tcp_flow_" + str(tcp_flow_id) + "_progress.csv",
        "pos_int,pos_int,pos_int"
    )
    num_entries = len(progress_csv_columns[0])
    tcp_flow_id_list = progress_csv_columns[0]
    time_ns_list = progress_csv_columns[1]
    progress_byte_list = progress_csv_columns[2]

    # TCP Flow ID list must be all exactly tcp_flow_id
    for i in tcp_flow_id_list:
        if i != tcp_flow_id:
            raise ValueError("The flow identifier does not match (it must be the same in the entire progress file)")

    # Add up all the progress made in that interval
    current_interval = (0, interval_ns, 0)
    intervals = []
    last_progress_byte = 0
    for i in range(num_entries):

        # Continue to fast-forward intervals until the next entry is in it
        while time_ns_list[i] >= current_interval[1]:
            intervals.append(current_interval)
            current_interval = (current_interval[1], current_interval[1] + interval_ns, 0)

        # Now it must be within current_interval
        current_interval = (
            current_interval[0],
            current_interval[1],
            current_interval[2] + progress_byte_list[i] - last_progress_byte
        )
        last_progress_byte = progress_byte_list[i]

    # Add the last interval if it is not empty
    if current_interval[2] != 0:
        intervals.append(current_interval)

    # Now go over the intervals
    #
    # Each interval [a, b] with progress c, gets converted into two points:
    # a, c
    # b - (small number), c
    #
    # This effectively creates a step function as a continuous line, which can then be plotted by gnuplot.
    #
    data_filename = data_out_dir + "/tcp_flow_" + str(tcp_flow_id) + "_rate_in_intervals.csv"
    with open(data_filename, "w+") as f_out:
        for i in range(len(intervals)):
            rate_megabit_per_s = intervals[i][2] / 125000.0 * (1e9 / interval_ns)
            f_out.write("%d,%.10f,%.10f\n" % (tcp_flow_id, intervals[i][0], rate_megabit_per_s))
            f_out.write("%d,%.10f,%.10f\n" % (tcp_flow_id, intervals[i][1] - 0.000001, rate_megabit_per_s))

    # Show what is produced
    print("Interval: " + str(interval_ns / 1000000.0) + " ms")
    print("Line format: [tcp_flow_id],[time_moment_ns],[rate in Mbps]")
    print("Produced: " + data_filename)


if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) != 4:
        print("Must supply exactly four arguments")
        print("Usage: python generate_tcp_flow_rate_csv.py [logs_ns3_dir] [data_out_dir] [tcp_flow_id] [interval_ns]")
        exit(1)
    else:
        generate_tcp_flow_rate_csv(
            args[0],
            args[1],
            int(args[2]),
            int(args[3])
        )
