/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "../test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/tcp-flow-scheduler.h"
#include "ns3/tcp-optimizer.h"

using namespace ns3;

const std::string tcp_optimizer_test_dir = ".tmp-tcp-optimizer-test";

void cleanup_tcp_optimizer_test() {
    remove_file_if_exists(tcp_optimizer_test_dir + "/config_ns3.properties");
    remove_file_if_exists(tcp_optimizer_test_dir + "/topology.properties.temp");
    remove_file_if_exists(tcp_optimizer_test_dir + "/tcp_flow_schedule.csv");
    remove_file_if_exists(tcp_optimizer_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(tcp_optimizer_test_dir + "/logs_ns3/timing_results.txt");
    remove_file_if_exists(tcp_optimizer_test_dir + "/logs_ns3/timing_results.csv");
    remove_dir_if_exists(tcp_optimizer_test_dir + "/logs_ns3");
    remove_dir_if_exists(tcp_optimizer_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class TcpOptimizerBasicTestCase : public TestCase
{
public:
    TcpOptimizerBasicTestCase () : TestCase ("tcp-optimizer basic") {};
    void DoRun () {

        mkdir_if_not_exists(tcp_optimizer_test_dir);
        std::ofstream config_file(tcp_optimizer_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream tcp_flow_schedule_file;
        tcp_flow_schedule_file.open (tcp_optimizer_test_dir + "/tcp_flow_schedule.csv");
        tcp_flow_schedule_file << "0,0,3,1000000,10000,," << std::endl; // Flow 0: 0 -> 3, 1MB, starting at t=10000ns
        tcp_flow_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (tcp_optimizer_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(tcp_optimizer_test_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Optimize TCP in only the basic sense
        TcpOptimizer::OptimizeBasic(basicSimulation);

        // Schedule UDP bursts
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Finalize the simulation
        basicSimulation->Finalize();

        cleanup_tcp_optimizer_test();
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpOptimizerWorstCaseRttTestCase : public TestCase
{
public:
    TcpOptimizerWorstCaseRttTestCase () : TestCase ("tcp-optimizer worst-case-rtt") {};
    void DoRun () {

        mkdir_if_not_exists(tcp_optimizer_test_dir);
        std::ofstream config_file(tcp_optimizer_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream tcp_flow_schedule_file;
        tcp_flow_schedule_file.open (tcp_optimizer_test_dir + "/tcp_flow_schedule.csv");
        tcp_flow_schedule_file << "0,0,3,1000000,10000,," << std::endl; // Flow 0: 0 -> 3, 1MB, starting at t=10000ns
        tcp_flow_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (tcp_optimizer_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(tcp_optimizer_test_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Optimize TCP
        TcpOptimizer::OptimizeUsingWorstCaseRtt(basicSimulation, topology->GetWorstCaseRttEstimateNs());

        // Schedule UDP bursts
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Finalize the simulation
        basicSimulation->Finalize();

        cleanup_tcp_optimizer_test();
    }
};

////////////////////////////////////////////////////////////////////////////////////////
