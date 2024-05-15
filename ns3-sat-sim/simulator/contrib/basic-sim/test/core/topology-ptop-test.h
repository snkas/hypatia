/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "../test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/queue.h"

using namespace ns3;

const std::string topology_ptop_test_dir = ".tmp-topology-ptop-test";

void prepare_topology_ptop_test_config() {
    mkdir_if_not_exists(topology_ptop_test_dir);
    std::ofstream config_file(topology_ptop_test_dir + "/config_ns3.properties");
    config_file << "simulation_end_time_ns=10000000000" << std::endl;
    config_file << "simulation_seed=123456789" << std::endl;
    config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
    config_file.close();
}

void cleanup_topology_ptop_test() {
    remove_file_if_exists(topology_ptop_test_dir + "/config_ns3.properties");
    remove_file_if_exists(topology_ptop_test_dir + "/topology.properties.temp");
    remove_file_if_exists(topology_ptop_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(topology_ptop_test_dir + "/logs_ns3/timing_results.txt");
    remove_file_if_exists(topology_ptop_test_dir + "/logs_ns3/timing_results.csv");
    remove_dir_if_exists(topology_ptop_test_dir + "/logs_ns3");
    remove_dir_if_exists(topology_ptop_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class TopologyPtopEmptyTestCase : public TestCase
{
public:
    TopologyPtopEmptyTestCase () : TestCase ("topology-ptop empty") {};
    void DoRun () {
        prepare_topology_ptop_test_config();
        
        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=0" << std::endl;
        topology_file << "num_undirected_edges=0" << std::endl;
        topology_file << "switches=set()" << std::endl;
        topology_file << "switches_which_are_tors=set()" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set()" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        
        ASSERT_EQUAL(topology->GetNumNodes(), 0);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 0);
        ASSERT_EQUAL(topology->GetSwitches().size(), 0);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 0);
        ASSERT_EQUAL(topology->GetServers().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 0);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 0);
        ASSERT_EQUAL(topology->GetEndpoints().size(), 0);
        
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();
    }
};

class TopologyPtopSingleTestCase : public TestCase
{
public:
    TopologyPtopSingleTestCase () : TestCase ("topology-ptop single") {};
    void DoRun () {
        prepare_topology_ptop_test_config();

        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-1: 10000)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=map(0->1: 100, 1->0: 100)" << std::endl;
        topology_file << "link_device_queue=map(0->1: drop_tail(100p), 1->0: drop_tail(100p))" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->1: disabled, 1->0: disabled)" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), 2);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 1);
        ASSERT_EQUAL(topology->GetSwitches().size(), 2);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 2);
        ASSERT_EQUAL(topology->GetServers().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 1);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 1);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[0].size(), 1);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[1].size(), 1);
        ASSERT_EQUAL(topology->GetEndpoints().size(), 2);

        // Check contents
        ASSERT_TRUE(topology->IsValidEndpoint(0));
        ASSERT_TRUE(topology->IsValidEndpoint(1));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), 0));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), 1));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), 0));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), 1));
        ASSERT_EQUAL(topology->GetUndirectedEdges()[0].first, 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges()[0].second, 1);
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(0, 1)));
        ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[0], 1));
        ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[1], 0));
        std::set<int64_t> endpoints = topology->GetEndpoints();
        ASSERT_TRUE(set_int64_contains(endpoints, 0));
        ASSERT_TRUE(set_int64_contains(endpoints, 1));

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

    }
};

class TopologyPtopTorTestCase : public TestCase
{
public:
    TopologyPtopTorTestCase () : TestCase ("topology-ptop tor") {};
    void DoRun () {
        prepare_topology_ptop_test_config();
        
        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=8" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(4)" << std::endl;
        topology_file << "switches_which_are_tors=set(4)" << std::endl;
        topology_file << "servers=set(0,1,2,3,5,6,7)" << std::endl;
        topology_file << "undirected_edges=set(0-4,1-4,2-4,3-4,4-5,4-6,4-7)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-4: 400,1-4: 500,2-4: 600,3-4: 700,4-5: 900,4-6: 10000,4-7: 11000)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=map(0->4: 2.8,1->4: 3.1,2->4: 3.4,3->4: 3.7,4->5: 4.7,4->6: 5.4,4->7: 6.1,4->0: 1.2,4->1: 1.9,4->2: 2.6,4->3: 3.3,5->4: 4.3,6->4: 4.6,7->4: 4.9)" << std::endl;
        topology_file << "link_device_queue=map(0->4: drop_tail(4p),1->4: drop_tail(4B),2->4: drop_tail(4p),3->4: drop_tail(4B),4->5: drop_tail(5p),4->6: drop_tail(6p),4->7: drop_tail(7p),4->0: drop_tail(77p),4->1: drop_tail(1p),4->2: drop_tail(2p),4->3: drop_tail(3p),5->4: drop_tail(4B),6->4: drop_tail(4p),7->4: drop_tail(4B))" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->4: default, 1->4: fq_codel_better_rtt, 2->4: disabled, 3->4: default, 5->4:disabled, 6->4:default, 7->4:fq_codel_better_rtt, 4->0: disabled, 4->1: fq_codel_better_rtt, 4->2: default, 4->3: disabled, 4->5: default, 4->6: disabled, 4->7: fq_codel_better_rtt)" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), 8);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 7);
        ASSERT_EQUAL(topology->GetSwitches().size(), 1);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 1);
        ASSERT_EQUAL(topology->GetServers().size(), 7);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 7);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 7);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 8);
        std::set<int64_t> endpoints = topology->GetEndpoints();
        ASSERT_EQUAL(endpoints.size(), 7);

        // Check contents
        for (int i = 0; i < 8; i++) {
            if (i == 4) {
                ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 7);
                ASSERT_FALSE(topology->IsValidEndpoint(i));
                ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));
                ASSERT_FALSE(set_int64_contains(endpoints, i));
                for (int j = 0; j < 8; j++) {
                    if (j != 4) {
                        ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], j));
                    }
                }
            } else {
                ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 1);
                ASSERT_TRUE(topology->IsValidEndpoint(i));
                ASSERT_FALSE(set_int64_contains(topology->GetSwitches(), i));
                ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetServers(), i));
                ASSERT_TRUE(set_int64_contains(endpoints, i));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 4));
                int a = i > 4 ? 4 : i;
                int b = i > 4 ? i : 4;
                ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(a, b)));
            }
        }

        // And now we are going to go test all the network devices installed and their channels in-between
        for (const std::pair<int64_t, int64_t>& edge : topology->GetUndirectedEdges()) {
            Ptr<PointToPointNetDevice> deviceAtoB = topology->GetNetDeviceForLink(edge);
            Ptr<PointToPointNetDevice> deviceBtoA = topology->GetNetDeviceForLink(std::make_pair(edge.second, edge.first));
            std::vector<std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>>> links_with_devices;
            links_with_devices.push_back(std::make_pair(edge, deviceAtoB));
            links_with_devices.push_back(std::make_pair(std::make_pair(edge.second, edge.first), deviceBtoA));
            for (std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>> link_and_device : links_with_devices) {

                // Under investigation
                std::pair<int64_t, int64_t> link = link_and_device.first;
                Ptr<PointToPointNetDevice> device = link_and_device.second;

                // Channel delay
                // In this test, the link delay is the two node identifiers
                // added together times 100, unless it's more than 10, then it's times 1000
                TimeValue delay_of_channel;
                device->GetChannel()->GetObject<PointToPointChannel>()->GetAttribute("Delay", delay_of_channel);
                ASSERT_EQUAL(delay_of_channel.Get().GetNanoSeconds(), (link.first + link.second >= 10 ? (link.first + link.second) * 10 : (link.first + link.second)) * 100);

                // Data rate
                // In this test, the data rate is the (first node identifier * 3 + second node identifier * 7) * 0.1
                DataRateValue data_rate;
                device->GetAttribute("DataRate", data_rate);
                ASSERT_EQUAL((int64_t) data_rate.Get().GetBitRate(), (link.first * 3 + link.second * 7) * 100000);

                // Queue
                // In this test, the even queue have packets as max size, the uneven have bytes
                // The max size is 77 if the second node identifier is 0, else the second node identifier
                PointerValue ptr;
                device->GetAttribute ("TxQueue", ptr);
                Ptr<Queue<Packet>> txQueue = ptr.Get<Queue<Packet>>();
                Ptr<QueueBase> abcQueue = txQueue->GetObject<QueueBase>();
                if (link.first % 2 == 0) {
                    ASSERT_EQUAL(abcQueue->GetMaxSize().GetUnit(), PACKETS);
                    NS_TEST_ASSERT_MSG_EQ(abcQueue->GetMaxSize().GetValue(), (link.second == 0 ? 77 : link.second), "For link: " + std::to_string(link.first) + " -> " + std::to_string(link.second));
                } else {
                    ASSERT_EQUAL(abcQueue->GetMaxSize().GetUnit(), BYTES);
                    ASSERT_EQUAL(abcQueue->GetMaxSize().GetValue(), (link.second == 0 ? 77 : link.second));
                }

                // Traffic control queueing discipline (based on formula (a * 2 + b * 7) % 3 = {0, 1, 2} = {fq_codel_better_rtt, default, disabled)
                Ptr<QueueDisc> queueDisc = topology->GetNodes().Get(link.first)->GetObject<TrafficControlLayer>()->GetRootQueueDiscOnDevice(device);
                if ((link.first * 2 + link.second * 7) % 3 == 0) {
                    // fq_codel_better_rtt
                    Ptr<FqCoDelQueueDisc> realDisc = queueDisc->GetObject<FqCoDelQueueDisc>();
                    ASSERT_NOT_EQUAL(!realDisc, 1);

                    // Improved interval (= RTT estimate)
                    StringValue interval_att;
                    realDisc->GetAttribute ("Interval", interval_att);
                    ASSERT_EQUAL(interval_att.Get(), std::to_string(topology->GetWorstCaseRttEstimateNs()) + "ns");

                    // Improved target (= RTT estimate / 20)
                    StringValue target_att;
                    realDisc->GetAttribute ("Target", target_att);
                    ASSERT_EQUAL(target_att.Get(), std::to_string(topology->GetWorstCaseRttEstimateNs() / 20) + "ns");

                } else if ((link.first * 2 + link.second * 7) % 3 == 1) {
                    // default (currently, fq codel is default)
                    ASSERT_NOT_EQUAL(!queueDisc->GetObject<FqCoDelQueueDisc>(), 1);

                } else {
                    // disabled
                    ASSERT_EQUAL(!queueDisc, 1);
                }

                // Check if the node identifiers here and on the other side match up
                ASSERT_EQUAL(device->GetNode()->GetId(), link.first);
                int64_t node_id_one = device->GetChannel()->GetObject<PointToPointChannel>()->GetDevice(0)->GetNode()->GetId();
                int64_t node_id_two = device->GetChannel()->GetObject<PointToPointChannel>()->GetDevice(1)->GetNode()->GetId();
                ASSERT_EQUAL(device->GetChannel()->GetObject<PointToPointChannel>()->GetNDevices(), 2);
                ASSERT_TRUE((node_id_one == link.first && node_id_two == link.second) || (node_id_one == link.second && node_id_two == link.first));

            }

        }

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

    }
};

class TopologyPtopLeafSpineTestCase : public TestCase
{
public:
    TopologyPtopLeafSpineTestCase () : TestCase ("topology-ptop leaf-spine") {};
    void DoRun () {
        prepare_topology_ptop_test_config();

        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=49" << std::endl;
        topology_file << "num_undirected_edges=72" << std::endl;
        topology_file << "switches=set(0,1,2,3,4,5,6,7,8,9,10,11,12)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3,4,5,6,7,8)" << std::endl;
        topology_file << "servers=set(13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48)" << std::endl;
        topology_file << "undirected_edges=set(0-9,0-10,0-11,0-12,0-13,0-14,0-15,0-16,1-9,1-10,1-11,1-12,1-17,1-18,1-19,1-20,2-9,2-10,2-11,2-12,2-21,2-22,2-23,2-24,3-9,3-10,3-11,3-12,3-25,3-26,3-27,3-28,4-9,4-10,4-11,4-12,4-29,4-30,4-31,4-32,5-9,5-10,5-11,5-12,5-33,5-34,5-35,5-36,6-9,6-10,6-11,6-12,6-37,6-38,6-39,6-40,7-9,7-10,7-11,7-12,7-41,7-42,7-43,7-44,8-9,8-10,8-11,8-12,8-45,8-46,8-47,8-48)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic values
        uint32_t num_spines = 4;
        uint32_t num_leafs = 9;
        uint32_t num_servers = num_spines * num_leafs;

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), num_spines + num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), num_spines * num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetSwitches().size(), num_spines + num_leafs);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), num_leafs);
        ASSERT_EQUAL(topology->GetServers().size(), num_servers);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), num_spines * num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), num_spines * num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), num_spines + num_leafs + num_servers);

        // Leafs
        for (int i = 0; i < 9; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 8);
            ASSERT_FALSE(topology->IsValidEndpoint(i));
            ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));

            // Links to the spines
            for (int j = 9; j < 13; j++) {
                ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(i, j)));
            }

            // Links to the servers
            for (int j = 13 + i * 4; j < 13 + (i + 1) * 4; j++) {
                ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(i, j)));
            }

        }

        // Spines
        for (int i = 9; i < 13; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 9);
            ASSERT_FALSE(topology->IsValidEndpoint(i));
            ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));
        }

        // Servers
        for (int i = 13; i < 49; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 1);
            ASSERT_TRUE(topology->IsValidEndpoint(i));
            ASSERT_FALSE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
            ASSERT_TRUE(set_int64_contains(topology->GetServers(), i));
            int tor = (i - 13) / 4;
            ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], tor));
            ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(tor, i)));
        }

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

    }
};

class TopologyPtopRingTestCase : public TestCase
{
public:
    TopologyPtopRingTestCase () : TestCase ("topology-ptop ring") {};
    void DoRun () {
        prepare_topology_ptop_test_config();

        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=100000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=0.1" << std::endl;
        topology_file << "link_device_queue=drop_tail(1p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=default" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), 4);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 4);
        ASSERT_EQUAL(topology->GetSwitches().size(), 4);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 2);
        ASSERT_EQUAL(topology->GetServers().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 4);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 4);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 4);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[0].size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[1].size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[2].size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[3].size(), 2);

        // Check contents
        ASSERT_TRUE(topology->IsValidEndpoint(0));
        ASSERT_FALSE(topology->IsValidEndpoint(1));
        ASSERT_FALSE(topology->IsValidEndpoint(2));
        ASSERT_TRUE(topology->IsValidEndpoint(3));
        for (int i = 0; i < 4; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 2);
            ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));
            if (i == 0 || i == 3) {
                ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 1));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 2));
            } else {
                ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 0));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 3));
            }
        }
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(0, 1)));
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(0, 2)));
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(1, 3)));
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(2, 3)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(1, 0)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(2, 0)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(3, 1)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(3, 2)));

        // Order of undirected edges list
        ASSERT_PAIR_EQUAL(topology->GetUndirectedEdges()[0], std::make_pair((int64_t) 0, (int64_t) 1));
        ASSERT_PAIR_EQUAL(topology->GetUndirectedEdges()[1], std::make_pair((int64_t) 0, (int64_t) 2));
        ASSERT_PAIR_EQUAL(topology->GetUndirectedEdges()[2], std::make_pair((int64_t) 1, (int64_t) 3));
        ASSERT_PAIR_EQUAL(topology->GetUndirectedEdges()[3], std::make_pair((int64_t) 2, (int64_t) 3));

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();
    }
};

class TopologyPtopInvalidTestCase : public TestCase
{
public:
    TopologyPtopInvalidTestCase () : TestCase ("topology-ptop invalid") {};
    void DoRun () {
        
        std::ofstream topology_file;
        Ptr<BasicSimulation> basicSimulation;

        // Incorrect number of nodes
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=1" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Incorrect number of edges
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=3" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Non-existent node id
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Not all node ids are covered
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=3" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Duplicate edge
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Duplicate node id
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Edge between server and non-ToR switch
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(1)" << std::endl;
        topology_file << "servers=set(2,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,3-0)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Edge between server and server
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(1)" << std::endl;
        topology_file << "servers=set(2,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Server marked as ToR
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2,3)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Servers and switches not distinct
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3,1)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Edge to itself
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-2)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Edge left out of bound
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-4)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Edge right out of bound
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,4-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Duplicate edges
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,02-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // This should be valid
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Invalid property
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_aretors=set(0,3)" << std::endl; // This one is invalid
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Undirected edge map, should work
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-1:100, 1-2:100, 2-3:100)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Invalid map order for undirected edge
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=map(1-0:100, 1-2:100, 2-3:100)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Duplicate in map for undirected edge
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-1:100, 1-2:100, 2-3:100, 00-1:100)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Undirected edge map has not-present edge
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-1:100, 1-2:100, 2-3:100, 0-3:50)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Undirected edge map does not fully cover
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-1:100, 1-2:100)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Directed edge map, should work
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=map(0->1:100, 1->2: 100, 2->3: 100, 1->0: 100, 2->1: 80, 3->2: 10)" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Directed edge map, duplicates
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=map(0->1:100, 1->2: 100, 2->3: 100, 1->0: 100, 2->1: 80, 3->2: 10, 0->01: 300)" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Directed edge map, unknown edge
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=map(0->1:100, 1->2: 100, 2->3: 100, 1->0: 100, 2->1: 80, 3->2: 10, 0->3: 88)" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Directed edge map, not all covered
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=map(0->1:100, 2->3: 100, 1->0: 100, 2->1: 80, 3->2: 10)" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Invalid queue type
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=non_existent_queue(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Invalid drop tail queue size
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(-8p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Invalid drop tail queue size
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100abc)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

        // Invalid traffic control queue disc
        prepare_topology_ptop_test_config();
        basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=some_invalid_qdisc_value_which_does_not_exist" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
