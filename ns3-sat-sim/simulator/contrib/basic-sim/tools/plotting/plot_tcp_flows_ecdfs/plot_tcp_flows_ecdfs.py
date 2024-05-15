import sys
import exputil
import numpy as np
from statsmodels.distributions.empirical_distribution import ECDF


def plot_tcp_flows_ecdfs(logs_ns3_dir, data_out_dir, pdf_out_dir):
    local_shell = exputil.LocalShell()

    # Check that all plotting files are available
    if not local_shell.file_exists("plot_tcp_flows_ecdf_fct.plt") \
       or not local_shell.file_exists("plot_tcp_flows_ecdf_avg_throughput.plt"):
        print("The gnuplot files are not present.")
        print("Are you executing this python file inside the plot_tcp_flows_ecdfs directory?")
        exit(1)

    # Create the output directories if they don't exist yet
    local_shell.make_full_dir(data_out_dir)
    local_shell.make_full_dir(pdf_out_dir)

    # Create rate file
    tcp_flows_csv_columns = exputil.read_csv_direct_in_columns(
        logs_ns3_dir + "/tcp_flows.csv",
        "idx_int,pos_int,pos_int,pos_int,pos_int,pos_int,pos_int,pos_int,string,string"
    )
    num_flows = len(tcp_flows_csv_columns[0])
    # flow_id_list = tcp_flows_csv_columns[0]
    # from_node_id_list = tcp_flows_csv_columns[1]
    # to_node_id_list = tcp_flows_csv_columns[2]
    size_byte_list = tcp_flows_csv_columns[3]
    # start_time_ns_list = tcp_flows_csv_columns[4]
    # end_time_ns_list = tcp_flows_csv_columns[5]
    duration_ns_list = tcp_flows_csv_columns[6]
    # amount_sent_ns_list = tcp_flows_csv_columns[7]
    finished_list = tcp_flows_csv_columns[8]
    # metadata_list = tcp_flows_csv_columns[9]

    # Retrieve FCTs
    num_finished = 0
    num_unfinished = 0
    fct_ms_list = []
    avg_throughput_megabit_per_s_list = []
    for i in range(num_flows):
        if finished_list[i] == "YES":
            fct_ms_list.append(duration_ns_list[i] / 1e6)
            avg_throughput_megabit_per_s_list.append(float(size_byte_list[i]) / float(duration_ns_list[i]) * 8000.0)
            num_finished += 1
        else:
            num_unfinished += 1

    # Exit if no TCP flows finished
    if num_finished == 0:
        raise ValueError("No TCP flows were finished so an ECDF could not be produced")

    # Now create ECDF for average throughput
    avg_throughput_megabit_per_s_ecdf = ECDF(avg_throughput_megabit_per_s_list)
    data_filename = data_out_dir + "/tcp_flows_ecdf_avg_throughput_megabit_per_s.csv"
    with open(data_filename, "w+") as f_out:
        for i in range(len(avg_throughput_megabit_per_s_ecdf.x)):
            f_out.write(
                str(avg_throughput_megabit_per_s_ecdf.x[i]) + "," + str(avg_throughput_megabit_per_s_ecdf.y[i]) + "\n"
            )

    # Plot ECDF of average throughput of each TCP flow
    pdf_filename = pdf_out_dir + "/plot_tcp_flows_ecdf_avg_throughput.pdf"
    plt_filename = "plot_tcp_flows_ecdf_avg_throughput.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    local_shell.remove("temp.plt")

    # Show final result
    print("Average throughput statistics:")
    print("  > Included (finished)....... %.2f%% (%d out of %d)"
          % (float(num_finished) / float(num_flows) * 100.0, num_finished, num_flows))
    print("  > Average throughput........ %.2f Mbit/s" % (np.mean(avg_throughput_megabit_per_s_list)))
    print("  > Minimum throughput........ %.2f Mbit/s (slowest)" % (np.min(avg_throughput_megabit_per_s_list)))
    print("  > 1th %%-tile throughput..... %.2f Mbit/s" % (np.percentile(avg_throughput_megabit_per_s_list, 1.0)))
    print("  > 10th %%-tile throughput.... %.2f Mbit/s" % (np.percentile(avg_throughput_megabit_per_s_list, 10.0)))
    print("  > Median throughput......... %.2f Mbit/s" % (np.percentile(avg_throughput_megabit_per_s_list, 50.0)))
    print("  > Maximum throughput........ %.2f Mbit/s (fastest)" % (np.max(avg_throughput_megabit_per_s_list)))
    print("")
    print("Produced ECDF data: " + data_filename)
    print("Produced ECDF plot: " + pdf_filename)

    # Now create ECDF for FCTs
    fct_ms_ecdf = ECDF(fct_ms_list)
    data_filename = data_out_dir + "/tcp_flows_ecdf_fct_ms.csv"
    with open(data_filename, "w+") as f_out:
        for i in range(len(fct_ms_ecdf.x)):
            f_out.write(str(fct_ms_ecdf.x[i]) + "," + str(fct_ms_ecdf.y[i]) + "\n")

    # Plot ECDF of FCTs
    pdf_filename = pdf_out_dir + "/plot_tcp_flows_ecdf_fct.pdf"
    plt_filename = "plot_tcp_flows_ecdf_fct.plt"
    local_shell.copy_file(plt_filename, "temp.plt")
    local_shell.sed_replace_in_file_plain("temp.plt", "[OUTPUT-FILE]", pdf_filename)
    local_shell.sed_replace_in_file_plain("temp.plt", "[DATA-FILE]", data_filename)
    local_shell.perfect_exec("gnuplot temp.plt")
    local_shell.remove("temp.plt")
    
    # Show final result
    print("FCT statistics:")
    print("  > Included (finished)... %.2f%% (%d out of %d)"
          % (float(num_finished) / float(num_flows) * 100.0, num_finished, num_flows))
    print("  > Average FCT........... %.2f ms" % (np.mean(fct_ms_list)))
    print("  > Minimum FCT........... %.2f ms (fastest)" % (np.min(fct_ms_list)))
    print("  > Median FCT............ %.2f ms" % (np.percentile(fct_ms_list, 50.0)))
    print("  > 90th %%-tile FCT....... %.2f ms" % (np.percentile(fct_ms_list, 90.0)))
    print("  > 99th %%-tile FCT....... %.2f ms" % (np.percentile(fct_ms_list, 99.0)))
    print("  > Maximum FCT........... %.2f ms (slowest)" % (np.max(fct_ms_list)))
    print("")
    print("Produced ECDF data: " + data_filename)
    print("Produced ECDF plot: " + pdf_filename)


def main():
    args = sys.argv[1:]
    if len(args) != 3:
        print("Must supply exactly three arguments")
        print("Usage: python plot_tcp_flows_ecdfs.py [logs_ns3_dir] [data_out_dir] [pdf_out_dir]")
        exit(1)
    else:
        plot_tcp_flows_ecdfs(
            args[0],
            args[1],
            args[2]
        )


if __name__ == "__main__":
    main()
