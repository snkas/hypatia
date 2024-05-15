/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/udp-burst-scheduler.h"
#include "ns3/tcp-optimizer.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/test.h"
#include "../test-helpers.h"
#include <iostream>
#include <fstream>

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndTestCase : public TestCase {
public:
    UdpBurstEndToEndTestCase(std::string s) : TestCase(s) {};
    const std::string temp_dir = ".tmp-udp-burst-end-to-end-test";

    void prepare_test_dir() {
        mkdir_if_not_exists(temp_dir);
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/udp_burst_schedule.csv");
    }

    void write_basic_config(int64_t simulation_end_time_ns, int64_t simulation_seed, std::string detailed_logging_enabled_for) {
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=set(" << detailed_logging_enabled_for << ")" << std::endl;
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

    void test_run_and_validate_udp_burst_logs(
            int64_t simulation_end_time_ns,
            std::string temp_dir, 
            std::vector<UdpBurstInfo> write_schedule,
            std::vector<double>& list_outgoing_rate_megabit_per_s,
            std::vector<double>& list_incoming_rate_megabit_per_s
    ) {

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts.csv");

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (temp_dir + "/udp_burst_schedule.csv");
        for (UdpBurstInfo entry : write_schedule) {
            schedule_file
                    << entry.GetUdpBurstId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetTargetRateMegabitPerSec() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetDurationNs() << ","
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
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(temp_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // For checking the sent / received amount
        std::vector<int64_t> udp_burst_sent_amount;
        std::vector<int64_t> udp_burst_received_amount;

        // Check udp_bursts_outgoing.csv
        std::vector<std::string> lines_outgoing_csv = read_file_direct(temp_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        ASSERT_EQUAL(lines_outgoing_csv.size(), write_schedule.size());
        int i = 0;
        for (std::string line : lines_outgoing_csv) {
            std::vector<std::string> line_spl = split_string(line, ",");
            ASSERT_EQUAL(line_spl.size(), 12);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), write_schedule[i].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), write_schedule[i].GetToNodeId());
            ASSERT_EQUAL(parse_positive_double(line_spl[3]), write_schedule[i].GetTargetRateMegabitPerSec());
            ASSERT_EQUAL(parse_positive_int64(line_spl[4]), write_schedule[i].GetStartTimeNs());
            ASSERT_EQUAL(parse_positive_int64(line_spl[5]), write_schedule[i].GetDurationNs());
            double outgoing_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[6]);
            double outgoing_rate_payload_megabit_per_s = parse_positive_double(line_spl[7]);
            int64_t sent_packets = parse_positive_int64(line_spl[8]);
            udp_burst_sent_amount.push_back(sent_packets);
            int64_t outgoing_sent_incl_headers_byte = parse_positive_int64(line_spl[9]);
            int64_t outgoing_sent_payload_only_byte = parse_positive_int64(line_spl[10]);
            ASSERT_EQUAL(outgoing_sent_payload_only_byte, sent_packets * 1472);
            ASSERT_EQUAL(outgoing_sent_incl_headers_byte, sent_packets * 1500);
            ASSERT_TRUE(outgoing_rate_payload_megabit_per_s >= outgoing_rate_incl_headers_megabit_per_s * 0.9);
            ASSERT_TRUE(outgoing_rate_incl_headers_megabit_per_s >= outgoing_rate_payload_megabit_per_s);
            ASSERT_EQUAL(line_spl[11], write_schedule[i].GetMetadata());
            list_outgoing_rate_megabit_per_s.push_back(outgoing_rate_incl_headers_megabit_per_s);
            i++;
        }

        // Check udp_bursts_incoming.csv
        std::vector<std::string> lines_incoming_csv = read_file_direct(temp_dir + "/logs_ns3/udp_bursts_incoming.csv");
        ASSERT_EQUAL(lines_incoming_csv.size(), write_schedule.size());
        i = 0;
        for (std::string line : lines_incoming_csv) {
            std::vector<std::string> line_spl = split_string(line, ",");
            ASSERT_EQUAL(line_spl.size(), 12);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), write_schedule[i].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), write_schedule[i].GetToNodeId());
            ASSERT_EQUAL(parse_positive_double(line_spl[3]), write_schedule[i].GetTargetRateMegabitPerSec());
            ASSERT_EQUAL(parse_positive_int64(line_spl[4]), write_schedule[i].GetStartTimeNs());
            ASSERT_EQUAL(parse_positive_int64(line_spl[5]), write_schedule[i].GetDurationNs());
            double incoming_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[6]);
            double incoming_rate_payload_megabit_per_s = parse_positive_double(line_spl[7]);
            int64_t received_packets = parse_positive_int64(line_spl[8]);
            udp_burst_received_amount.push_back(received_packets);
            int64_t incoming_received_incl_headers_byte = parse_positive_int64(line_spl[9]);
            int64_t incoming_received_payload_only_byte = parse_positive_int64(line_spl[10]);
            ASSERT_EQUAL(incoming_received_payload_only_byte, received_packets * 1472);
            ASSERT_EQUAL(incoming_received_incl_headers_byte, received_packets * 1500);
            ASSERT_TRUE(incoming_rate_payload_megabit_per_s >= incoming_rate_incl_headers_megabit_per_s * 0.9);
            ASSERT_TRUE(incoming_rate_incl_headers_megabit_per_s >= incoming_rate_payload_megabit_per_s);
            ASSERT_EQUAL(line_spl[11], write_schedule[i].GetMetadata());
            list_incoming_rate_megabit_per_s.push_back(incoming_rate_incl_headers_megabit_per_s);
            i++;
        }

        // Check udp_bursts_outgoing.txt
        std::vector<std::string> lines_outgoing_txt = read_file_direct(temp_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        ASSERT_EQUAL(lines_outgoing_txt.size(), write_schedule.size() + 1);
        i = 0;
        for (std::string line : lines_outgoing_txt) {
            if (i == 0) {
                ASSERT_EQUAL(
                        line,
                        "UDP burst ID    From      To        Target rate         Start time      Duration        Outgoing rate (w/ headers)  Outgoing rate (payload)     Packets sent    Data sent (w/headers)       Data sent (payload)         Metadata"
                        );
            } else {
                int j = i - 1;
                std::vector<std::string> line_spl;
                std::istringstream iss(line);
                for (std::string s; iss >> s;) {
                    line_spl.push_back(s);
                }
                ASSERT_TRUE(line_spl.size() == 18 || line_spl.size() == 19);
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), j);
                ASSERT_EQUAL(parse_positive_int64(line_spl[1]), write_schedule[j].GetFromNodeId());
                ASSERT_EQUAL(parse_positive_int64(line_spl[2]), write_schedule[j].GetToNodeId());
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), write_schedule[j].GetTargetRateMegabitPerSec(), 0.01);
                ASSERT_EQUAL(line_spl[4], "Mbit/s");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[5]), nanosec_to_millisec(write_schedule[j].GetStartTimeNs()), 0.01);
                ASSERT_EQUAL(line_spl[6], "ms");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(write_schedule[j].GetDurationNs()), 0.01);
                ASSERT_EQUAL(line_spl[8], "ms");
                double outgoing_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[9]);
                ASSERT_EQUAL(line_spl[10], "Mbit/s");
                double outgoing_rate_payload_megabit_per_s = parse_positive_double(line_spl[11]);
                ASSERT_EQUAL(line_spl[12], "Mbit/s");
                ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s[j], outgoing_rate_incl_headers_megabit_per_s, 0.01);
                int64_t packets_sent = parse_positive_int64(line_spl[13]);
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[14]), byte_to_megabit(packets_sent * 1500), 0.01);
                ASSERT_EQUAL(line_spl[15], "Mbit");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[16]), byte_to_megabit(packets_sent * 1472), 0.01);
                ASSERT_EQUAL(line_spl[17], "Mbit");
                ASSERT_TRUE(outgoing_rate_payload_megabit_per_s >= outgoing_rate_incl_headers_megabit_per_s * 0.9);
                ASSERT_TRUE(outgoing_rate_incl_headers_megabit_per_s >= outgoing_rate_payload_megabit_per_s);
                if (line_spl.size() == 19) {
                    ASSERT_EQUAL(line_spl[18], write_schedule[j].GetMetadata());
                }
            }
            i++;
        }

        // Check udp_bursts_incoming.txt
        std::vector<std::string> lines_incoming_txt = read_file_direct(temp_dir + "/logs_ns3/udp_bursts_incoming.txt");
        ASSERT_EQUAL(lines_incoming_txt.size(), write_schedule.size() + 1);
        i = 0;
        for (std::string line : lines_incoming_txt) {
            if (i == 0) {
                ASSERT_EQUAL(
                        line,
                        "UDP burst ID    From      To        Target rate         Start time      Duration        Incoming rate (w/ headers)  Incoming rate (payload)     Packets received   Data received (w/headers)   Data received (payload)     Metadata"
                );
            } else {
                int j = i - 1;
                std::vector<std::string> line_spl;
                std::istringstream iss(line);
                for (std::string s; iss >> s;) {
                    line_spl.push_back(s);
                }
                ASSERT_TRUE(line_spl.size() == 18 || line_spl.size() == 19);
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), j);
                ASSERT_EQUAL(parse_positive_int64(line_spl[1]), write_schedule[j].GetFromNodeId());
                ASSERT_EQUAL(parse_positive_int64(line_spl[2]), write_schedule[j].GetToNodeId());
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), write_schedule[j].GetTargetRateMegabitPerSec(), 0.01);
                ASSERT_EQUAL(line_spl[4], "Mbit/s");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[5]), nanosec_to_millisec(write_schedule[j].GetStartTimeNs()), 0.01);
                ASSERT_EQUAL(line_spl[6], "ms");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(write_schedule[j].GetDurationNs()), 0.01);
                ASSERT_EQUAL(line_spl[8], "ms");
                double incoming_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[9]);
                ASSERT_EQUAL(line_spl[10], "Mbit/s");
                double incoming_rate_payload_megabit_per_s = parse_positive_double(line_spl[11]);
                ASSERT_EQUAL(line_spl[12], "Mbit/s");
                ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s[j], incoming_rate_incl_headers_megabit_per_s, 0.01);
                int64_t packets_received = parse_positive_int64(line_spl[13]);
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[14]), byte_to_megabit(packets_received * 1500), 0.01);
                ASSERT_EQUAL(line_spl[15], "Mbit");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[16]), byte_to_megabit(packets_received * 1472), 0.01);
                ASSERT_EQUAL(line_spl[17], "Mbit");
                ASSERT_TRUE(incoming_rate_payload_megabit_per_s >= incoming_rate_incl_headers_megabit_per_s * 0.9);
                ASSERT_TRUE(incoming_rate_incl_headers_megabit_per_s >= incoming_rate_payload_megabit_per_s);
                if (line_spl.size() == 19) {
                    ASSERT_EQUAL(line_spl[18], write_schedule[j].GetMetadata());
                }
            }
            i++;
        }

        // Check the precise outgoing / incoming logs for each burst
        for (UdpBurstInfo entry : write_schedule) {

            // Outgoing
            std::vector<std::string> lines_precise_outgoing_csv = read_file_direct(temp_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_outgoing.csv");
            ASSERT_EQUAL(lines_precise_outgoing_csv.size(), (size_t) udp_burst_sent_amount.at(entry.GetUdpBurstId()));
            int j = 0;
            for (std::string line : lines_precise_outgoing_csv) {
                std::vector <std::string> line_spl = split_string(line, ",");
                ASSERT_EQUAL(line_spl.size(), 3);
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetUdpBurstId());
                ASSERT_EQUAL(parse_positive_int64(line_spl[1]), j);
                ASSERT_EQUAL(parse_positive_int64(line_spl[2]), entry.GetStartTimeNs() + j * std::ceil(1500.0 / (entry.GetTargetRateMegabitPerSec() / 8000.0)));
                j += 1;
            }

            // Incoming
            std::vector<std::string> lines_precise_incoming_csv = read_file_direct(temp_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_incoming.csv");
            ASSERT_EQUAL(lines_precise_incoming_csv.size(), (size_t) udp_burst_received_amount.at(entry.GetUdpBurstId()));
            std::set<int64_t> already_seen_seqs;
            int prev_timestamp_ns = 0;
            for (std::string line : lines_precise_incoming_csv) {
                std::vector <std::string> line_spl = split_string(line, ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Must be correct UDP burst ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetUdpBurstId());

                // We can only check that the sequence number has not arrived before
                int64_t seq = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(already_seen_seqs.find(seq) == already_seen_seqs.end());
                already_seen_seqs.insert(seq);

                // And that the timestamps are at least weakly ascending
                int64_t timestamp = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(timestamp >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp;
            }

        }

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_incoming.txt");
        for (UdpBurstInfo entry : write_schedule) {
            remove_file_if_exists(temp_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_outgoing.csv");
            remove_file_if_exists(temp_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_incoming.csv");
        }
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndOneToOneEqualStartTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndOneToOneEqualStartTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end 1-to-1 equal-start") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0,1");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 9, 1000000000, 3000000000, "", "abc"));
        schedule.push_back(UdpBurstInfo(1, 1, 0, 9, 1000000000, 3000000000, "", ""));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // As they are started at the same point and should have the same behavior, progress should be equal
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 2);
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.at(0), list_outgoing_rate_megabit_per_s.at(1));
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 8.5);
        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 2);
        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.at(0), list_incoming_rate_megabit_per_s.at(1));
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 8.5);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndSingleOverflowTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndSingleOverflowTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end single-overflow") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 25, 1000000000, 3000000000, "", "abc"));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // Not everything will arrive
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 1);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 24.5);
        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 1);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 9.5);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndDoubleEnoughTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndDoubleEnoughTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end double-enough") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0,1");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 3, 1000000000, 3000000000, "", "abc"));
        schedule.push_back(UdpBurstInfo(1, 0, 1, 4, 1000000000, 3000000000, "", "abc"));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // They will share the overflow
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 2.5 && list_outgoing_rate_megabit_per_s.at(0) <= 3.1);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(1) >= 3.5 && list_outgoing_rate_megabit_per_s.at(1) <= 4.1);

        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 2.5 && list_incoming_rate_megabit_per_s.at(0) <= 3.1);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(1) >= 3.5 && list_incoming_rate_megabit_per_s.at(1) <= 4.1);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndDoubleOverflowTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndDoubleOverflowTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end double-overflow") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0,1");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 15, 1000000000, 3000000000, "", "abc"));
        schedule.push_back(UdpBurstInfo(1, 0, 1, 10, 1000000000, 3000000000, "", "abc"));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // They will share the overflow
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 14.5);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(1) >= 9.5);

        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 5.0);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(1) >= 2.5);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndNotEnabledTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndNotEnabledTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end not-enabled") {};

    void DoRun () {

        // Run directory
        prepare_test_dir();

        // Config file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=false" << std::endl;
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
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
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

class UdpBurstEndToEndInvalidLoggingIdTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndInvalidLoggingIdTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end invalid-logging-id") {};

    void DoRun () {

        // Run directory
        prepare_test_dir();

        // Config file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=set(0,1)" << std::endl; // Invalid entry: 1
        config_file.close();

        // One flow
        std::ofstream schedule_file;
        schedule_file.open (temp_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,0,1,100,0,10000,," << std::endl;
        schedule_file.close();

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
        ASSERT_EXCEPTION(UdpBurstScheduler(basicSimulation, topology));
        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////
