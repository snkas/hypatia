/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/exp-util.h"
#include "ns3/udp-burst-schedule-reader.h"
#include "ns3/topology-ptop.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "../test-helpers.h"

using namespace ns3;

const std::string udp_burst_schedule_reader_test_dir = ".tmp-udp-burst-scheduler-reader-test";

void prepare_udp_burst_schedule_reader_test_config() {
    mkdir_if_not_exists(udp_burst_schedule_reader_test_dir);
}

void cleanup_udp_burst_schedule_reader_test() {
    remove_file_if_exists(udp_burst_schedule_reader_test_dir + "/config_ns3.properties");
    remove_file_if_exists(udp_burst_schedule_reader_test_dir + "/topology.properties");
    remove_file_if_exists(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
    remove_file_if_exists(udp_burst_schedule_reader_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(udp_burst_schedule_reader_test_dir + "/logs_ns3/timing_results.txt");
    remove_file_if_exists(udp_burst_schedule_reader_test_dir + "/logs_ns3/timing_results.csv");
    remove_dir_if_exists(udp_burst_schedule_reader_test_dir + "/logs_ns3");
    remove_dir_if_exists(udp_burst_schedule_reader_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstScheduleReaderNormalTestCase : public TestCase
{
public:
    UdpBurstScheduleReaderNormalTestCase () : TestCase ("udp-burst-schedule-reader normal") {};

    void DoRun () {
        prepare_udp_burst_schedule_reader_test_config();

        // Normal

        std::ofstream config_file(udp_burst_schedule_reader_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream schedule_file(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,0,1,1000.3,123456,20000,a=c,test14" << std::endl;
        schedule_file << "1,7,3,1,7488338,1356567,a=b2," << std::endl;
        schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (udp_burst_schedule_reader_test_dir + "/topology.properties");
        topology_file << "num_nodes=8" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(0,1,2,3,4,5,6,7)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3,4,5,6,7)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,3-4,4-5,5-6,6-7)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(udp_burst_schedule_reader_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        std::vector<UdpBurstInfo> schedule = read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000000);

        ASSERT_EQUAL(schedule.size(), 2);

        ASSERT_EQUAL(schedule[0].GetUdpBurstId(), 0);
        ASSERT_EQUAL(schedule[0].GetFromNodeId(), 0);
        ASSERT_EQUAL(schedule[0].GetToNodeId(), 1);
        ASSERT_EQUAL(schedule[0].GetTargetRateMegabitPerSec(), 1000.3);
        ASSERT_EQUAL(schedule[0].GetStartTimeNs(), 123456);
        ASSERT_EQUAL(schedule[0].GetDurationNs(), 20000);
        ASSERT_EQUAL(schedule[0].GetAdditionalParameters(), "a=c");
        ASSERT_EQUAL(schedule[0].GetMetadata(), "test14");

        ASSERT_EQUAL(schedule[1].GetUdpBurstId(), 1);
        ASSERT_EQUAL(schedule[1].GetFromNodeId(), 7);
        ASSERT_EQUAL(schedule[1].GetToNodeId(), 3);
        ASSERT_EQUAL(schedule[1].GetTargetRateMegabitPerSec(), 1);
        ASSERT_EQUAL(schedule[1].GetStartTimeNs(), 7488338);
        ASSERT_EQUAL(schedule[1].GetDurationNs(), 1356567);
        ASSERT_EQUAL(schedule[1].GetAdditionalParameters(), "a=b2");
        ASSERT_EQUAL(schedule[1].GetMetadata(), "");

        // Empty

        std::ofstream schedule_file_empty(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file_empty.close();
        std::vector<UdpBurstInfo> schedule_empty = read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000);
        ASSERT_EQUAL(schedule_empty.size(), 0);

        basicSimulation->Finalize();
        cleanup_udp_burst_schedule_reader_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstScheduleReaderInvalidTestCase : public TestCase
{
public:
    UdpBurstScheduleReaderInvalidTestCase () : TestCase ("udp-burst-schedule-reader invalid") {};

    void DoRun () {
        prepare_udp_burst_schedule_reader_test_config();

        std::ofstream schedule_file;
        std::vector<UdpBurstInfo> schedule;

        std::ofstream config_file(udp_burst_schedule_reader_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream topology_file;
        topology_file.open (udp_burst_schedule_reader_test_dir + "/topology.properties");
        topology_file << "num_nodes=5" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3,4)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,3,4)" << std::endl; // Only 2 cannot be endpoint
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,3-4)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(udp_burst_schedule_reader_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Non-existent file
        ASSERT_EXCEPTION(read_udp_burst_schedule("does-not-exist-temp.file", topology, 10000000));
        
        // Normal
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        schedule = read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000);

        // Source = Destination
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,3,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Invalid source (out of range)
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,9,3,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Invalid destination (out of range)
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,9,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Invalid source (not a ToR)
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,2,3,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Invalid destination (not a ToR)
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,2,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Not ascending UDP burst ID
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "1,3,1,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Negative UDP burst rate
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,-33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Zero UDP burst rate
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,0,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Not enough values
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,33,100002,2000009,xyz" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Negative time
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,33,-1,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(schedule = read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Just normal ordered with equal start time
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file << "1,3,1,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        schedule = read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000);

        // Not ordered time
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,33,100002,2000009,xyz,abc" << std::endl;
        schedule_file << "1,3,1,33,100001,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        // Exceeding time
        schedule_file = std::ofstream(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,3,1,33,10000000,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_burst_schedule(udp_burst_schedule_reader_test_dir + "/udp_burst_schedule.csv", topology, 10000000));

        basicSimulation->Finalize();
        cleanup_udp_burst_schedule_reader_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
