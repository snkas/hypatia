import sys
from exputil import *


def plot_link_utilization(logs_ns3_dir, data_out_dir, pdf_out_dir, from_node_id, to_node_id):
    local_shell = LocalShell()

    # Check that the plotting file is available
    if not local_shell.file_exists("plot_time_vs_link_utilization.plt"):
        print("The gnuplot file is not present.")
        print("Are you executing this python file inside the plot_link_utilization directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)

    # Read in CSV
    utilization_compressed_csv_columns = read_csv_direct_in_columns(
        logs_ns3_dir + "/link_utilization.csv",
        "pos_int,pos_int,pos_int,pos_int,pos_int"
    )
    num_entries = len(utilization_compressed_csv_columns[0])
    from_list = utilization_compressed_csv_columns[0]
    to_list = utilization_compressed_csv_columns[1]
    range_start_ns_list = utilization_compressed_csv_columns[2]
    range_end_ns_list = utilization_compressed_csv_columns[3]
    busy_ns_list = utilization_compressed_csv_columns[4]

    # Create step plottable utilization file
    #
    # Each interval [a, b] with busy time c, gets converted into two points:
    # a, c / (b - a)
    # b - (small number), c / (b - a)
    #
    # This effectively creates a step function as a continuous line, which can then be plotted by gnuplot.
    #
    data_filename = "%s/link_utilization_%d_to_%d_fraction_in_intervals.csv" % (data_out_dir, from_node_id, to_node_id)
    with open(data_filename, "w+") as f_out:
        for i in range(num_entries):
            if from_list[i] == from_node_id and to_list[i] == to_node_id:
                utilization_fraction = busy_ns_list[i] / (range_end_ns_list[i] - range_start_ns_list[i])
                f_out.write("%.10f,%.10f\n" % (range_start_ns_list[i], utilization_fraction))
                f_out.write("%.10f,%.10f\n" % (range_end_ns_list[i] - 0.000001, utilization_fraction))

    # Plot time vs. utilization
    pdf_filename = pdf_out_dir + "/plot_link_utilization_" + str(from_node_id) + "_to_" + str(to_node_id) + ".pdf"
    plt_filename = "plot_time_vs_link_utilization.plt"
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
        print("Usage: python plot_link_utilization.py [logs_ns3 directory] [data_out_dir] [pdf_out_dir]"
              " [from_node_id] [to_node_id]")
        exit(1)
    else:
        plot_link_utilization(
            args[0],
            args[1],
            args[2],
            int(args[3]),
            int(args[4])
        )


if __name__ == "__main__":
    main()
