import sys
import exputil


def sum_up_into_intervals(interval_ns, time_ns_list, data_list):
    current_interval = (0, interval_ns, 0)
    intervals = []
    for i in range(len(time_ns_list)):

        # Continue to fast-forward intervals until the next entry is in it
        while time_ns_list[i] >= current_interval[1]:
            intervals.append(current_interval)
            current_interval = (current_interval[1], current_interval[1] + interval_ns, 0)

        # Now it must be within current_interval
        current_interval = (
            current_interval[0],
            current_interval[1],
            current_interval[2] + data_list[i]
        )

    # Add the last interval
    intervals.append(current_interval)

    return intervals


def generate_udp_burst_sent_amount_csv(data_out_dir, udp_burst_id, outgoing_time_ns_list):
    filename = data_out_dir + "/udp_burst_" + str(udp_burst_id) + "_total_sent_byte.csv"
    with open(filename, "w+") as f_out:
        prev_amount = 0
        for i in range(len(outgoing_time_ns_list)):
            prev_amount += 1500
            f_out.write("%d,%f,%d\n" % (udp_burst_id, outgoing_time_ns_list[i], prev_amount))
    print("Produced data file: " + filename)
    return filename


def generate_udp_burst_arrived_amount_csv(data_out_dir, udp_burst_id, incoming_time_ns_list):
    filename = data_out_dir + "/udp_burst_" + str(udp_burst_id) + "_total_arrived_byte.csv"
    with open(filename, "w+") as f_out:
        prev_amount = 0
        for i in range(len(incoming_time_ns_list)):
            prev_amount += 1500
            f_out.write("%d,%f,%d\n" % (udp_burst_id, incoming_time_ns_list[i], prev_amount))
    print("Produced data file: " + filename)
    return filename


def generate_udp_burst_sent_rate_csv(data_out_dir, udp_burst_id, outgoing_time_ns_list, interval_ns):
    intervals = sum_up_into_intervals(interval_ns, outgoing_time_ns_list, [1500] * len(outgoing_time_ns_list))
    filename = data_out_dir + "/udp_burst_" + str(udp_burst_id) + "_sent_rate_megabit_per_s_in_intervals.csv"
    with open(filename, "w+") as f_out:
        for i in range(len(intervals)):
            rate_megabit_per_s = intervals[i][2] * 8000.0 / interval_ns 
            f_out.write("%d,%.10f,%.10f\n" % (udp_burst_id, intervals[i][0], rate_megabit_per_s))
            f_out.write("%d,%.10f,%.10f\n" % (udp_burst_id, intervals[i][1] - 0.000001, rate_megabit_per_s))
    print("Produced data file: " + filename)
    return filename


def generate_udp_burst_arrived_rate_csv(data_out_dir, udp_burst_id, incoming_time_ns_list, interval_ns):
    intervals = sum_up_into_intervals(interval_ns, incoming_time_ns_list, [1500] * len(incoming_time_ns_list))
    filename = data_out_dir + "/udp_burst_" + str(udp_burst_id) + "_arrived_rate_megabit_per_s_in_intervals.csv"
    with open(filename, "w+") as f_out:
        for i in range(len(intervals)):
            rate_megabit_per_s = intervals[i][2] * 8000.0 / interval_ns
            f_out.write("%d,%.10f,%.10f\n" % (udp_burst_id, intervals[i][0], rate_megabit_per_s))
            f_out.write("%d,%.10f,%.10f\n" % (udp_burst_id, intervals[i][1] - 0.000001, rate_megabit_per_s))
    print("Produced data file: " + filename)
    return filename


def generate_udp_burst_latency_csv(data_out_dir, udp_burst_id, outgoing_time_ns_list,
                                   incoming_seq_no_list, incoming_time_ns_list):
    filename = data_out_dir + "/udp_burst_" + str(udp_burst_id) + "_one_way_latency_ns.csv"
    with open(filename, "w+") as f_out:
        arrived = {}
        for i in range(len(incoming_time_ns_list)):
            arrived[incoming_seq_no_list[i]] = incoming_time_ns_list[i]
        for i in range(len(outgoing_time_ns_list)):
            if i in arrived:
                f_out.write("%d,%f,%d\n" % (
                    udp_burst_id,
                    outgoing_time_ns_list[i],
                    arrived[i] - outgoing_time_ns_list[i])
                )
    print("Produced data file: " + filename)
    return filename


def plot_udp_burst(logs_ns3_dir, data_out_dir, pdf_out_dir, udp_burst_id, interval_ns):
    local_shell = exputil.LocalShell()

    # Check that all plotting files are available
    if (
        not local_shell.file_exists("plot_udp_burst_time_vs_amount_sent.plt") or
        not local_shell.file_exists("plot_udp_burst_time_vs_amount_arrived.plt") or
        not local_shell.file_exists("plot_udp_burst_time_vs_sent_rate.plt") or
        not local_shell.file_exists("plot_udp_burst_time_vs_arrived_rate.plt") or
        not local_shell.file_exists("plot_udp_burst_time_vs_one_way_latency.plt")
    ):
        print("The gnuplot files are not present.")
        print("Are you executing this python file inside the plot_udp_burst directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)

    # Read in CSV of the outgoing packets
    outgoing_csv_columns = exputil.read_csv_direct_in_columns(
        logs_ns3_dir + "/udp_burst_" + str(udp_burst_id) + "_outgoing.csv",
        "pos_int,pos_int,pos_int"
    )
    outgoing_num_entries = len(outgoing_csv_columns[0])
    outgoing_udp_burst_id_list = outgoing_csv_columns[0]
    if outgoing_udp_burst_id_list != [udp_burst_id] * outgoing_num_entries:
        raise ValueError("Mismatched UDP burst ID in outgoing data")
    outgoing_seq_no_list = outgoing_csv_columns[1]
    if outgoing_seq_no_list != list(range(outgoing_num_entries)):
        raise ValueError("Not all outgoing sequence numbers are incremented")
    outgoing_time_ns_list = outgoing_csv_columns[2]

    # Read in CSV of the incoming packets
    incoming_csv_columns = exputil.read_csv_direct_in_columns(
        logs_ns3_dir + "/udp_burst_" + str(udp_burst_id) + "_incoming.csv",
        "pos_int,pos_int,pos_int"
    )
    incoming_num_entries = len(incoming_csv_columns[0])
    incoming_udp_burst_id_list = incoming_csv_columns[0]
    if incoming_udp_burst_id_list != [udp_burst_id] * incoming_num_entries:
        raise ValueError("Mismatched UDP burst ID in incoming data")
    incoming_seq_no_list = incoming_csv_columns[1]
    incoming_time_ns_list = incoming_csv_columns[2]

    # Generate the data files
    filename_sent_amount_byte = generate_udp_burst_sent_amount_csv(
        data_out_dir, udp_burst_id, outgoing_time_ns_list
    )
    filename_arrived_amount_byte = generate_udp_burst_arrived_amount_csv(
        data_out_dir, udp_burst_id, incoming_time_ns_list
    )
    filename_sent_rate_megabit_per_s = generate_udp_burst_sent_rate_csv(
        data_out_dir, udp_burst_id, outgoing_time_ns_list, interval_ns
    )
    filename_arrived_rate_megabit_per_s = generate_udp_burst_arrived_rate_csv(
        data_out_dir, udp_burst_id, incoming_time_ns_list, interval_ns
    )
    filename_latency_ns = generate_udp_burst_latency_csv(
        data_out_dir, udp_burst_id, outgoing_time_ns_list, incoming_seq_no_list, incoming_time_ns_list
    )

    # Plot time vs. amount sent
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_amount_sent_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_amount_sent.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", filename_sent_amount_byte)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. amount arrived
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_amount_arrived_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_amount_arrived.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", filename_arrived_amount_byte)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. sent rate
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_sent_rate_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_sent_rate.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", filename_sent_rate_megabit_per_s)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. arrived rate
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_arrived_rate_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_arrived_rate.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", filename_arrived_rate_megabit_per_s)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # Plot time vs. latency
    pdf_filename = pdf_out_dir + "/plot_udp_burst_time_vs_one_way_latency_" + str(udp_burst_id) + ".pdf"
    plt_filename = "plot_udp_burst_time_vs_one_way_latency.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", filename_latency_ns)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")


def main():
    args = sys.argv[1:]
    if len(args) != 5:
        print("Must supply exactly five arguments")
        print("Usage: python plot_udp_burst.py [logs_ns3_dir] [data_out_dir] [pdf_out_dir] [udp_burst_id]"
              " [interval_ns (for rates)]")
        exit(1)
    else:
        plot_udp_burst(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4])
        )


if __name__ == "__main__":
    main()
