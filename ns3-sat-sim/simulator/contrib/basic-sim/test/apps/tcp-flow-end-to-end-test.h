/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/tcp-flow-scheduler.h"
#include "ns3/tcp-optimizer.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/test.h"
#include "../test-helpers.h"
#include <iostream>
#include <fstream>

using namespace ns3;

class BeforeRunOperation {
public:
    virtual void operation(Ptr<TopologyPtop> topology) = 0;
};

class BeforeRunOperationNothing : public BeforeRunOperation {
public:
    void operation(Ptr<TopologyPtop> topology) {
        // Nothing
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndTestCase : public TestCase {
public:
    TcpFlowEndToEndTestCase(std::string s) : TestCase(s) {};
    const std::string temp_dir = ".tmp-tcp-flow-end-to-end-test";

    void prepare_test_dir() {
        mkdir_if_not_exists(temp_dir);
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/tcp_flow_schedule.csv");
    }

    void write_basic_config(int64_t simulation_end_time_ns, int64_t simulation_seed, uint32_t num_tcp_flows) {
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        std::string ids = "";
        for (size_t i = 0; i < num_tcp_flows; i++) {
            if (i != 0) {
                ids = ids + ",";
            }
            ids = ids + std::to_string(i);
        }
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=set(" << ids << ")" << std::endl;
        config_file.close();
    }

    void write_single_topology(double link_device_data_rate_megabit_per_s, int64_t link_channel_delay_ns) {
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=" << link_channel_delay_ns << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=" << link_device_data_rate_megabit_per_s << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
    }

    void test_run_and_validate_tcp_flow_logs(int64_t simulation_end_time_ns, std::string temp_dir, std::vector<TcpFlowScheduleEntry> write_schedule, std::vector<int64_t>& end_time_ns_list, std::vector<int64_t>& sent_byte_list, std::vector<std::string>& finished_list, BeforeRunOperation* beforeRunOperation) {

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.csv");

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (temp_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpOptimizer::OptimizeUsingWorstCaseRtt(basicSimulation, topology->GetWorstCaseRttEstimateNs());
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        beforeRunOperation->operation(topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(temp_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // Check tcp_flows.csv
        std::vector<std::string> lines_csv = read_file_direct(temp_dir + "/logs_ns3/tcp_flows.csv");
        ASSERT_EQUAL(lines_csv.size(), write_schedule.size());
        int i = 0;
        for (std::string line : lines_csv) {
            std::vector<std::string> line_spl = split_string(line, ",");
            ASSERT_EQUAL(line_spl.size(), 10);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), write_schedule[i].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), write_schedule[i].GetToNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[3]), write_schedule[i].GetSizeByte());
            ASSERT_EQUAL(parse_positive_int64(line_spl[4]), write_schedule[i].GetStartTimeNs());
            int64_t end_time_ns = parse_positive_int64(line_spl[5]);
            ASSERT_TRUE(end_time_ns >= 0 && end_time_ns <= simulation_end_time_ns);
            ASSERT_EQUAL(parse_positive_int64(line_spl[6]), end_time_ns - write_schedule[i].GetStartTimeNs());
            int64_t sent_byte = parse_positive_int64(line_spl[7]);
            ASSERT_TRUE(sent_byte >= 0 && sent_byte <= simulation_end_time_ns);
            ASSERT_TRUE(line_spl[8] == "YES" || line_spl[8] == "NO_ONGOING" || line_spl[8] == "NO_CONN_FAIL" || line_spl[8] == "NO_ERR_CLOSE" || line_spl[8] == "NO_BAD_CLOSE");
            ASSERT_EQUAL(line_spl[9], write_schedule[i].GetMetadata());
            end_time_ns_list.push_back(end_time_ns);
            sent_byte_list.push_back(sent_byte);
            finished_list.push_back(line_spl[8]);
            i++;
        }

        // Check tcp_flows.txt
        std::vector<std::string> lines_txt = read_file_direct(temp_dir + "/logs_ns3/tcp_flows.txt");
        ASSERT_EQUAL(lines_txt.size(), write_schedule.size() + 1);
        i = 0;
        for (std::string line : lines_txt) {
            if (i == 0) {
                ASSERT_EQUAL(
                        line,
                        "TCP Flow ID     Source    Target    Size            Start time (ns)   End time (ns)     Duration        Sent            Progress     Avg. rate       Finished?     Metadata"
                        );
            } else {
                int j = i - 1;
                std::vector<std::string> line_spl;
                std::istringstream iss(line);
                for (std::string s; iss >> s;) {
                    line_spl.push_back(s);
                }
                ASSERT_TRUE(line_spl.size() == 15 || line_spl.size() == 16);
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), j);
                ASSERT_EQUAL(parse_positive_int64(line_spl[1]), write_schedule[j].GetFromNodeId());
                ASSERT_EQUAL(parse_positive_int64(line_spl[2]), write_schedule[j].GetToNodeId());
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), byte_to_megabit(write_schedule[j].GetSizeByte()), 0.01);
                ASSERT_EQUAL(line_spl[4], "Mbit");
                ASSERT_EQUAL(parse_positive_int64(line_spl[5]), write_schedule[j].GetStartTimeNs());
                ASSERT_EQUAL(parse_positive_int64(line_spl[6]), end_time_ns_list[j]);
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(end_time_ns_list[j] - write_schedule[j].GetStartTimeNs()), 0.01);
                ASSERT_EQUAL(line_spl[8], "ms");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[9]), byte_to_megabit(sent_byte_list[j]), 0.01);
                ASSERT_EQUAL(line_spl[10], "Mbit");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[11].substr(0, line_spl[11].size() - 1)), sent_byte_list[j] * 100.0 / write_schedule[j].GetSizeByte(), 0.1);
                ASSERT_EQUAL(line_spl[11].substr(line_spl[11].size() - 1, line_spl[11].size()), "%");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[12]), byte_to_megabit(sent_byte_list[j]) / nanosec_to_sec(end_time_ns_list[j] - write_schedule[j].GetStartTimeNs()), 0.1);
                ASSERT_EQUAL(line_spl[13], "Mbit/s");
                ASSERT_TRUE(line_spl[14] == "YES" || line_spl[14] == "NO_ONGOING" || line_spl[14] == "NO_CONN_FAIL" || line_spl[14] == "NO_ERR_CLOSE" || line_spl[14] == "NO_BAD_CLOSE");
                ASSERT_EQUAL(line_spl[14], finished_list[j]);
                if (line_spl.size() == 16) {
                    ASSERT_EQUAL(line_spl[15], write_schedule[j].GetMetadata());
                }
            }
            i++;
        }

        // Now go over all the detailed logs
        for (TcpFlowScheduleEntry& entry : write_schedule) {

            // TCP congestion window
            // tcp_flow_[id]_cwnd.csv
            std::vector<std::string> lines_cwnd_csv = read_file_direct(temp_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_cwnd.csv");
            int64_t prev_timestamp_ns = 0;
            int64_t prev_cwnd_byte = -1;
            for (size_t i = 0; i < lines_cwnd_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_cwnd_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // Congestion window has to be positive and different
                int64_t cwnd_byte = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(cwnd_byte != prev_cwnd_byte || (i == lines_cwnd_csv.size() - 1 && cwnd_byte == prev_cwnd_byte));
                prev_cwnd_byte = cwnd_byte;
            }

            // TCP connection progress
            // tcp_flow_[id]_progress.csv
            std::vector<std::string> lines_progress_csv = read_file_direct(temp_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_progress.csv");
            prev_timestamp_ns = 0;
            int64_t prev_progress_byte = 0;
            for (size_t i = 0; i < lines_progress_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_progress_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // Progress has to be positive and ascending
                int64_t progress_byte = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(progress_byte >= prev_progress_byte);
                prev_progress_byte = progress_byte;
            }
            ASSERT_EQUAL(prev_progress_byte, sent_byte_list.at(entry.GetTcpFlowId())); // Progress must be equal to the sent amount reported in the end

            // TCP RTT
            // tcp_flow_[id]_rtt.csv
            std::vector<std::string> lines_rtt_csv = read_file_direct(temp_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_rtt.csv");
            prev_timestamp_ns = 0;
            int64_t prev_rtt_ns = -1;
            for (size_t i = 0; i < lines_rtt_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_rtt_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // RTT has to be positive and different
                int64_t rtt_ns = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(rtt_ns >= 0);
                ASSERT_TRUE(rtt_ns != prev_rtt_ns || (i == lines_rtt_csv.size() - 1 && rtt_ns == prev_rtt_ns));
                prev_rtt_ns = rtt_ns;
            }

        }

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.txt");
        for (size_t i = 0; i < write_schedule.size(); i++) {
            remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
        }
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneToOneEqualStartTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneEqualStartTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 equal-start") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, 2);
        write_single_topology(10.0, 100000);

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 1000000, 0, "", "abc"));
        schedule.push_back(TcpFlowScheduleEntry(1, 1, 0, 1000000, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // As they are started at the same point and should have the same behavior, progress should be equal
        ASSERT_EQUAL(end_time_ns_list[0], end_time_ns_list[1]);
        ASSERT_EQUAL(sent_byte_list[0], sent_byte_list[1]);
        ASSERT_EQUAL(finished_list[0], "YES");
        ASSERT_EQUAL(finished_list[1], "YES");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneToOneSimpleStartTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneSimpleStartTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 simple") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 100.0 Mbit/s, 10 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, 1);
        write_single_topology(100.0, 10000);

        // One flow
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 300, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // As they are started at the same point and should have the same behavior, progress should be equal
        int expected_end_time_ns = 0;
        expected_end_time_ns += 4 * 10000; // SYN, SYN+ACK, ACK+DATA, FIN (last ACK+FIN is not acknowledged, and all the data is already ACKed)
        // 1 byte / 100 Mbit/s = 80 ns to transmit 1 byte
        expected_end_time_ns += 58 * 80;  // SYN = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 16 (TCP SYN options: wndscale: 3, timestamp: 10, SACK-perm: 2 -> in 4 byte steps so padded to 16) = 58 bytes
        expected_end_time_ns += 58 * 80;  // SYN+ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 16 (TCP SYN options) = 58 bytes
        expected_end_time_ns += 54 * 80;  // ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option: timestamp: 10 -> in 4 byte steps so padded to 12) = 54 bytes
        expected_end_time_ns += 354 * 80; // ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option) + Data (300) = 354 bytes
        expected_end_time_ns += 54 * 80;  // ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option) = 54 bytes
        expected_end_time_ns += 54 * 80;  // FIN+ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option)  = 54 bytes
        ASSERT_EQUAL_APPROX(end_time_ns_list[0], expected_end_time_ns, 6); // 6 transmissions for which the rounding can be down
        ASSERT_EQUAL(300, sent_byte_list[0]);
        ASSERT_EQUAL(finished_list[0], "YES");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneToOneApartStartTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneApartStartTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 apart-start") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 10000000000;

        // One-to-one, 10s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 654321, 2);
        write_single_topology(10.0, 100000);

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 1000000, 0, "", "first"));
        schedule.push_back(TcpFlowScheduleEntry(1, 0, 1, 1000000, 5000000000, "", "second"));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // As they are started without any interference, should have same completion
        ASSERT_EQUAL(end_time_ns_list[0], end_time_ns_list[1] - 5000000000);
        ASSERT_EQUAL(sent_byte_list[0], sent_byte_list[1]);
        ASSERT_EQUAL(finished_list[0], "YES");
        ASSERT_EQUAL(finished_list[1], "YES");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndEcmpSimpleTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndEcmpSimpleTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end ecmp-simple") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 20);
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=30.0" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=fq_codel_better_rtt" << std::endl;
        topology_file.close();

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        for (int i = 0; i < 20; i++) {
            schedule.push_back(TcpFlowScheduleEntry(i, 0, 3, 1000000000, 0, "", ""));
        }

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // All are too large to end, and they must consume bandwidth of both links
        int64_t byte_sum = 0;
        for (int i = 0; i < 20; i++) {
            ASSERT_EQUAL(end_time_ns_list[i], simulation_end_time_ns);
            ASSERT_EQUAL(finished_list[i], "NO_ONGOING");
            byte_sum += sent_byte_list[i];
        }
        ASSERT_TRUE(byte_to_megabit(byte_sum) / nanosec_to_sec(simulation_end_time_ns) >= 45.0); // At least 45 Mbit/s, given that probability of all of them going up is 0.5^8

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndEcmpRemainTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndEcmpRemainTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end ecmp-remain") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 1);
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 3, 1000000000, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // Can only consume the bandwidth of one path, the one it is hashed to
        ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
        double rate_Mbps = byte_to_megabit(sent_byte_list[0]) / nanosec_to_sec(end_time_ns_list[0]);
        ASSERT_TRUE(rate_Mbps >= 25.0 && rate_Mbps <= 30.0); // Somewhere between 25 and 30
        ASSERT_EQUAL(finished_list[0], "NO_ONGOING");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterSpecificDrop: public ArbiterPtop
{
public:
    int m_counter = 0;
    int m_drop_threshold;

    ArbiterSpecificDrop(Ptr<Node> this_node, NodeContainer nodes, Ptr<TopologyPtop> topology, int drop_threshold) : ArbiterPtop(this_node, nodes, topology) {
        // Empty
        m_drop_threshold = drop_threshold;
    }

    int32_t TopologyPtopDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            const std::set<int64_t>& neighbor_node_ids,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) {
        if (source_node_id == 1) {
            if (m_counter >= m_drop_threshold) {
                return -1;
            } else {
                m_counter += 1;
                if (target_node_id == 3) {
                    return 3;
                } else {
                    throw std::runtime_error("Not possible");
                }
            }
        } else {
            if (source_node_id == 3 && target_node_id == 1) {
                return 1;
            } else if (source_node_id == 0 && target_node_id == 3) {
                return 3;
            } else if (source_node_id == 3 && target_node_id == 0) {
                return 0;
            } else {
                throw std::runtime_error("Not possible");
            }
        }
    }

    std::string StringReprOfForwardingState() {
        return "";
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class BeforeRunOperationDropper : public BeforeRunOperation {
public:
    int m_drop_threshold;
    BeforeRunOperationDropper(int drop_threshold) {
        m_drop_threshold = drop_threshold;
    }
    void operation(Ptr<TopologyPtop> topology) {
        Ptr<ArbiterSpecificDrop> dropper = CreateObject<ArbiterSpecificDrop>(topology->GetNodes().Get(2), topology->GetNodes(), topology, m_drop_threshold);
        topology->GetNodes().Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(dropper);
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneDropOneNotTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneDropOneNotTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end one-drop-one-not") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 2);
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(2)" << std::endl;
        topology_file << "switches_which_are_tors=set(2)" << std::endl;
        topology_file << "servers=set(0, 1, 3)" << std::endl;
        topology_file << "undirected_edges=set(0-2,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Two flows
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 3, 1000000000, 0, "", ""));
        schedule.push_back(TcpFlowScheduleEntry(1, 1, 3, 1000000000, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationDropper op(0);
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // Can only consume the bandwidth of one path, the one it is hashed to
        ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
        ASSERT_EQUAL(end_time_ns_list[1], simulation_end_time_ns);
        double rate_Mbps = byte_to_megabit(sent_byte_list[0]) / nanosec_to_sec(end_time_ns_list[0]);
        ASSERT_TRUE(rate_Mbps >= 25.0 && rate_Mbps <= 30.0); // Somewhere between 25 and 30
        ASSERT_EQUAL(sent_byte_list[1], 0);
        ASSERT_EQUAL(finished_list[0], "NO_ONGOING");
        ASSERT_EQUAL(finished_list[1], "NO_ONGOING");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndConnFailTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndConnFailTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end conn-fail") {};

    void DoRun () {
        prepare_test_dir();

        // One-to-one, 10s, 30.0 Mbit/s, 200 microsec delay
        int64_t simulation_end_time_ns = 50000000000;
        write_basic_config(simulation_end_time_ns, 123456, 1);
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(2)" << std::endl;
        topology_file << "switches_which_are_tors=set(2)" << std::endl;
        topology_file << "servers=set(0, 1, 3)" << std::endl;
        topology_file << "undirected_edges=set(0-2,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Two flows
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 1, 3, 1000000000, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationDropper op(0);
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // Can only consume the bandwidth of one path, the one it is hashed to
        ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
        ASSERT_EQUAL(sent_byte_list[0], 0);
        ASSERT_EQUAL(finished_list[0], "NO_CONN_FAIL");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndPrematureCloseTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndPrematureCloseTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end premature-close") {};

    void DoRun () {

        for (int num_packets_allowed = 1; num_packets_allowed < 5; num_packets_allowed++) {

            prepare_test_dir();

            // One-to-one, 10s, 30.0 Mbit/s, 200 microsec delay
            int64_t simulation_end_time_ns = 200000000000;
            write_basic_config(simulation_end_time_ns, 123456, 1);
            std::ofstream topology_file;
            topology_file.open(temp_dir + "/topology.properties");
            topology_file << "num_nodes=4" << std::endl;
            topology_file << "num_undirected_edges=3" << std::endl;
            topology_file << "switches=set(2)" << std::endl;
            topology_file << "switches_which_are_tors=set(2)" << std::endl;
            topology_file << "servers=set(0, 1, 3)" << std::endl;
            topology_file << "undirected_edges=set(0-2,1-2,2-3)" << std::endl;
            topology_file << "link_channel_delay_ns=200000" << std::endl;
            topology_file << "link_device_data_rate_megabit_per_s=30" << std::endl;
            topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
            topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
            topology_file.close();

            // Two flows
            std::vector <TcpFlowScheduleEntry> schedule;
            schedule.push_back(TcpFlowScheduleEntry(0, 1, 3, 1000000000, 0, "", ""));

            // Perform the run
            std::vector <int64_t> end_time_ns_list;
            std::vector <int64_t> sent_byte_list;
            std::vector <std::string> finished_list;
            BeforeRunOperationDropper op(num_packets_allowed);
            test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, temp_dir, schedule, end_time_ns_list,
                                                sent_byte_list, finished_list, &op);

            // Can only consume the bandwidth of one path, the one it is hashed to
            ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
            ASSERT_EQUAL(sent_byte_list[0], std::max(num_packets_allowed - 2, 0) * 1380);
            ASSERT_EQUAL(finished_list[0], "NO_ERR_CLOSE");

        }

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndNonExistentRunDirTestCase : public TestCase
{
public:
    TcpFlowEndToEndNonExistentRunDirTestCase () : TestCase ("tcp-flow-end-to-end non-existent-run-dir") {};

    void DoRun () {
        ASSERT_EXCEPTION(BasicSimulation simulation("path/to/non/existent/run/dir"));
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndNotEnabledTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndNotEnabledTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end not-enabled") {};

    void DoRun () {

        // Run directory
        prepare_test_dir();

        // Config file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_tcp_flow_scheduler=false" << std::endl;
        config_file.close();

        // Topology
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(2)" << std::endl;
        topology_file << "switches_which_are_tors=set(2)" << std::endl;
        topology_file << "servers=set(0, 1, 3)" << std::endl;
        topology_file << "undirected_edges=set(0-2,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndInvalidLoggingIdTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndInvalidLoggingIdTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end invalid-logging-id") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 2); // One too many
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (temp_dir + "/tcp_flow_schedule.csv");
        schedule_file << "0,0,3,1000000000,0,," << std::endl;
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ASSERT_EXCEPTION(TcpFlowScheduler(basicSimulation, topology));
        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
