/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "../test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/ptop-link-queue-tracker-helper.h"
#include "ns3/udp-burst-scheduler.h"

using namespace ns3;

const std::string ptop_queue_test_dir = ".tmp-ptop-queue-test";

void cleanup_ptop_queue_test() {
    remove_file_if_exists(ptop_queue_test_dir + "/config_ns3.properties");
    remove_file_if_exists(ptop_queue_test_dir + "/topology.properties.temp");
    remove_file_if_exists(ptop_queue_test_dir + "/udp_burst_schedule.csv");
    remove_file_if_exists(ptop_queue_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(ptop_queue_test_dir + "/logs_ns3/timing_results.txt");
    remove_file_if_exists(ptop_queue_test_dir + "/logs_ns3/timing_results.csv");
    remove_file_if_exists(ptop_queue_test_dir + "/logs_ns3/link_queue_pkt.csv");
    remove_file_if_exists(ptop_queue_test_dir + "/logs_ns3/link_queue_byte.csv");
    remove_dir_if_exists(ptop_queue_test_dir + "/logs_ns3");
    remove_dir_if_exists(ptop_queue_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class PtopLinkQueueSimpleTestCase : public TestCase
{
public:
    PtopLinkQueueSimpleTestCase () : TestCase ("ptop-link-queue simple") {};
    void DoRun () {

        mkdir_if_not_exists(ptop_queue_test_dir);
        std::ofstream config_file(ptop_queue_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
        config_file << "enable_link_queue_tracking=true" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (ptop_queue_test_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file << "1,2,3,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "2,3,2,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "3,1,3,120,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (ptop_queue_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(ptop_queue_test_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install link queue trackers
        PtopLinkQueueTrackerHelper linkQueueTrackerHelper = PtopLinkQueueTrackerHelper(basicSimulation, topology); // Requires enable_link_queue_tracking=true

        // Run simulation
        basicSimulation->Run();

        // Write link queue results
        linkQueueTrackerHelper.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Directed edge list
        std::vector<std::pair<int64_t, int64_t>> dir_a_b_list;
        for (std::pair<int64_t, int64_t> a_b : topology->GetUndirectedEdges()) {
            dir_a_b_list.push_back(a_b);
            dir_a_b_list.push_back(std::make_pair(a_b.second, a_b.first));
        }
        std::sort(dir_a_b_list.begin(), dir_a_b_list.end());

        // Both link queue files need to be checked, and their checking is nearly identical
        for (size_t file_choice = 0;  file_choice < 2; file_choice++) {

            // Packets or bytes, all the checks are the same basically
            std::vector<std::string> lines_csv;
            if (file_choice == 0) {
                lines_csv = read_file_direct(ptop_queue_test_dir + "/logs_ns3/link_queue_pkt.csv");
            } else {
                lines_csv = read_file_direct(ptop_queue_test_dir + "/logs_ns3/link_queue_byte.csv");
            }

            // State variables when checking
            int64_t current_from_to_idx = -1;
            std::pair<int64_t, int64_t> current_from_to = std::make_pair(-1, -1);
            int64_t previous_interval_end_ns = 0;
            int64_t prev_num_packets_or_bytes = -1;

            // Go over every line
            for (size_t line_i = 0; line_i < lines_csv.size(); line_i++) {
                std::string line = lines_csv[line_i];
                std::vector<std::string> comma_split = split_string(line, ",", 5);
                int64_t from_node_id = parse_positive_int64(comma_split[0]);
                int64_t to_node_id = parse_positive_int64(comma_split[1]);
                int64_t interval_start_ns = parse_positive_int64(comma_split[2]);
                int64_t interval_end_ns = parse_positive_int64(comma_split[3]);
                int64_t num_packets_or_bytes = parse_positive_int64(comma_split[4]);

                // From-to has to be ordered and matching
                if (from_node_id != current_from_to.first || to_node_id != current_from_to.second) {
                    previous_interval_end_ns = 0;
                    prev_num_packets_or_bytes = -1;
                    current_from_to_idx += 1;
                    current_from_to = dir_a_b_list[current_from_to_idx];
                    ASSERT_EQUAL(interval_start_ns, 0); // The first entry must have zero time
                }
                ASSERT_TRUE(from_node_id == current_from_to.first);
                ASSERT_TRUE(to_node_id == current_from_to.second);

                // Time intervals must match up
                ASSERT_EQUAL(interval_start_ns, previous_interval_end_ns);
                previous_interval_end_ns = interval_end_ns;

                // Value must be different
                ASSERT_NOT_EQUAL(prev_num_packets_or_bytes, num_packets_or_bytes);
                prev_num_packets_or_bytes = num_packets_or_bytes;

                // Now we check that the number of packets/bytes make sense given the traffic started
                if (from_node_id == 1 && to_node_id == 3) {
                    if (interval_start_ns >= 251000000) {
                        ASSERT_TRUE(num_packets_or_bytes > 0);
                    }
                    if (interval_start_ns >= 400000000) {
                        if (file_choice == 0) {
                            ASSERT_TRUE(num_packets_or_bytes >= 99);
                        } else {
                            ASSERT_TRUE(num_packets_or_bytes >= 148698);
                        }
                    }
                } else {
                    ASSERT_EQUAL(num_packets_or_bytes, 0); // All other links cannot have a queue building up
                }

            }

        }

        cleanup_ptop_queue_test();
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopLinkQueueNotEnabledTestCase : public TestCase
{
public:
    PtopLinkQueueNotEnabledTestCase () : TestCase ("ptop-link-queue not-enabled") {};
    void DoRun () {

        mkdir_if_not_exists(ptop_queue_test_dir);
        std::ofstream config_file(ptop_queue_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
        config_file << "enable_link_queue_tracking=false" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (ptop_queue_test_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (ptop_queue_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(ptop_queue_test_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install queue trackers
        PtopLinkQueueTrackerHelper linkQueueTracker = PtopLinkQueueTrackerHelper(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write queue results
        linkQueueTracker.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Nothing should have been logged
        ASSERT_FALSE(file_exists(ptop_queue_test_dir + "/logs_ns3/link_queue_pkt.csv"));
        ASSERT_FALSE(file_exists(ptop_queue_test_dir + "/logs_ns3/link_queue_byte.csv"));

        cleanup_ptop_queue_test();
    }
};

////////////////////////////////////////////////////////////////////////////////////////
