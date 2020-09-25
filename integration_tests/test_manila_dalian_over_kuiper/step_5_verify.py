import exputil

local_shell = exputil.LocalShell()


def assert_true(val):
    if not val:
        raise ValueError("Failed verification")
        
        
# Generated satellite network state is there
for gen_data_subdir in [
    "temp/gen_data/reduced_kuiper_630_algorithm_free_one_only_over_isls",
    "temp/gen_data/reduced_kuiper_630_algorithm_free_gs_one_sat_many_only_over_isls"
]:
    for i in range(0, 200000000000, 100000000):
        assert_true(local_shell.file_exists(gen_data_subdir + "/description.txt"))
        assert_true(local_shell.file_exists(gen_data_subdir + "/ground_stations.txt"))
        assert_true(local_shell.file_exists(gen_data_subdir + "/gsl_interfaces_info.txt"))
        assert_true(local_shell.file_exists(gen_data_subdir + "/isls.txt"))
        assert_true(local_shell.file_exists(gen_data_subdir + "/tles.txt"))
        assert_true(local_shell.file_exists(gen_data_subdir + "/dynamic_state_100ms_for_200s/fstate_" + str(i) + ".txt"))
        assert_true(local_shell.file_exists(gen_data_subdir + "/dynamic_state_100ms_for_200s/gsl_if_bandwidth_" + str(i) + ".txt"))

# Runs are finished
assert_true(local_shell.read_file("temp/runs/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/logs_ns3/finished.txt").strip() == "Yes")
assert_true(local_shell.read_file("temp/runs/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/logs_ns3/finished.txt").strip() == "Yes")

# Data is there
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_cwnd.csv"))
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_progress.csv"))
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_rate_in_intervals.csv"))
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_rtt.csv"))
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_cwnd.csv"))
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_progress.csv"))
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_rate_in_intervals.csv"))
assert_true(local_shell.file_exists("temp/data/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/tcp_flow_0_rtt.csv"))

# Plots are there
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_cwnd_0.pdf"))
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_progress_0.pdf"))
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_rate_0.pdf"))
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_one_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_rtt_0.pdf"))
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_cwnd_0.pdf"))
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_progress_0.pdf"))
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_rate_0.pdf"))
assert_true(local_shell.file_exists("temp/pdf/kuiper_630_isls_sat_many_17_to_18_with_TcpNewReno_at_10_Mbps/plot_tcp_flow_time_vs_rtt_0.pdf"))

print("Verification completed")
