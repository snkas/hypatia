import sys
from exputil import *


def plot_link_queue(logs_ns3_dir, data_out_dir, pdf_out_dir, from_node_id, to_node_id):
    local_shell = LocalShell()

    # Check that the plotting file is available
    if (
        not local_shell.file_exists("plot_time_vs_link_queue_pkt.plt") or
        not local_shell.file_exists("plot_time_vs_link_queue_byte.plt")
    ):
        print("The gnuplot file(s) is not present.")
        print("Are you executing this python file inside the plot_link_queue directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)

    # Read in CSVs

    # Packets (only for parsing to check format)
    queue_pkt_csv_columns = read_csv_direct_in_columns(
        logs_ns3_dir + "/link_queue_pkt.csv",
        "pos_int,pos_int,pos_int,pos_int,pos_int"
    )
    queue_pkt_num_entries = len(queue_pkt_csv_columns[0])
    queue_pkt_from_list = queue_pkt_csv_columns[0]
    queue_pkt_to_list = queue_pkt_csv_columns[1]
    queue_pkt_interval_start_ns_list = queue_pkt_csv_columns[2]
    queue_pkt_interval_end_ns_list = queue_pkt_csv_columns[3]
    queue_pkt_size_pkt_list = queue_pkt_csv_columns[4]

    # Byte (only for parsing to check format)
    queue_byte_csv_columns = read_csv_direct_in_columns(
        logs_ns3_dir + "/link_queue_byte.csv",
        "pos_int,pos_int,pos_int,pos_int,pos_int"
    )
    queue_byte_num_entries = len(queue_byte_csv_columns[0])
    queue_byte_from_list = queue_byte_csv_columns[0]
    queue_byte_to_list = queue_byte_csv_columns[1]
    queue_byte_interval_start_ns_list = queue_byte_csv_columns[2]
    queue_byte_interval_end_ns_list = queue_byte_csv_columns[3]
    queue_byte_size_byte_list = queue_byte_csv_columns[4]

    # For packets
    data_filename = "%s/link_queue_%d_to_%d_pkt_in_intervals.csv" % (data_out_dir, from_node_id, to_node_id)
    with open(data_filename, "w+") as f_out:
        for i in range(queue_pkt_num_entries):
            if queue_pkt_from_list[i] == from_node_id and queue_pkt_to_list[i] == to_node_id:
                f_out.write("%.10f,%.10f\n"
                            % (queue_pkt_interval_start_ns_list[i], queue_pkt_size_pkt_list[i]))
                f_out.write("%.10f,%.10f\n"
                            % (queue_pkt_interval_end_ns_list[i] - 0.000001, queue_pkt_size_pkt_list[i]))

    # Plot time vs. queue (packets)
    pdf_filename = pdf_out_dir + "/plot_link_queue_pkt_" + str(from_node_id) + "_to_" + str(to_node_id) + ".pdf"
    plt_filename = "plot_time_vs_link_queue_pkt.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")

    # For byte
    data_filename = "%s/link_queue_%d_to_%d_byte_in_intervals.csv" % (data_out_dir, from_node_id, to_node_id)
    with open(data_filename, "w+") as f_out:
        for i in range(queue_byte_num_entries):
            if queue_byte_from_list[i] == from_node_id and queue_byte_to_list[i] == to_node_id:
                f_out.write("%.10f,%.10f\n"
                            % (queue_byte_interval_start_ns_list[i], queue_byte_size_byte_list[i]))
                f_out.write("%.10f,%.10f\n"
                            % (queue_byte_interval_end_ns_list[i] - 0.000001, queue_byte_size_byte_list[i]))

    # Plot time vs. queue (byte)
    pdf_filename = pdf_out_dir + "/plot_link_queue_byte_" + str(from_node_id) + "_to_" + str(to_node_id) + ".pdf"
    plt_filename = "plot_time_vs_link_queue_byte.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    print("Produced plot: " + pdf_filename)
    local_shell.remove("temp.plt")


def main():
    args = sys.argv[1:]
    if len(args) != 5:
        print("Must supply exactly five arguments")
        print("Usage: python plot_link_queue.py [logs_ns3 directory] [data_out_dir] [pdf_out_dir]"
              " [from_node_id] [to_node_id]")
        exit(1)
    else:
        plot_link_queue(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4])
        )


if __name__ == "__main__":
    main()
