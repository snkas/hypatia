/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/tcp-flow-scheduler.h"
#include "ns3/tcp-optimizer.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/udp-burst-helper.h"
#include "ns3/test.h"
#include "../test-helpers.h"
#include <iostream>
#include <fstream>

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class ManualEndToEndTestCase : public TestCase {
public:
    ManualEndToEndTestCase () : TestCase ("manual-end-to-end") {};
    const std::string temp_dir = ".tmp-manual-end-to-end-test";

    void DoRun () {

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");

        // Basic configuration
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=2000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file.close();

        // Topology
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpOptimizer::OptimizeBasic(basicSimulation);

        // Install a UDP burst client on all
        UdpBurstHelper udpBurstHelper(1026, basicSimulation->GetLogsDir());
        ApplicationContainer udpApp = udpBurstHelper.Install(topology->GetNodes());
        udpApp.Start(Seconds(0.0));

        // UDP burst info entry
        UdpBurstInfo udpBurstInfo(
                0,
                0,
                1,
                15,
                1000,
                700000000,
                "abc",
                "def"
        );

        // Register all bursts being sent from there and being received
        udpApp.Get(0)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                udpBurstInfo,
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                true
        );
        udpApp.Get(1)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                udpBurstInfo,
                true
        );

        // Install flow sink on all
        TcpFlowSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 1024));
        ApplicationContainer app = sink.Install(topology->GetNodes());
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(1900000000));

        // 0 --> 1
        TcpFlowSendHelper source0(
                "ns3::TcpSocketFactory",
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                1000000,
                0,
                true,
                basicSimulation->GetLogsDir(),
                ""
        );
        app = source0.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(1000000000));

        // 1 --> 0
        TcpFlowSendHelper source1(
                "ns3::TcpSocketFactory",
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                89999,
                1,
                true,
                basicSimulation->GetLogsDir(),
                ""
        );
        app = source1.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(7000));
        app.Stop(NanoSeconds(1000000000));

        // Install echo server on all nodes
        UdpRttServerHelper echoServerHelper(1025);
        app = echoServerHelper.Install(topology->GetNodes());
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(600000000));

        // Pinging 0 --> 1
        UdpRttClientHelper source_ping0(
                topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                1025,
                0,
                1
        );
        source_ping0.SetAttribute("Interval", TimeValue(NanoSeconds(100000000)));
        app = source_ping0.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(1000000000));

        // Pinging 1 --> 0
        UdpRttClientHelper source_ping1(
                topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                1025,
                1,
                0
        );
        source_ping1.SetAttribute("Interval", TimeValue(NanoSeconds(100000000)));
        app = source_ping1.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(1000000000));

        // Run simulation
        basicSimulation->Run();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(temp_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // Check UDP burst completion information
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> outgoing_0_info = udpApp.Get(0)->GetObject<UdpBurstApplication>()->GetOutgoingBurstsInformation();
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> outgoing_1_info = udpApp.Get(1)->GetObject<UdpBurstApplication>()->GetOutgoingBurstsInformation();
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_0_info = udpApp.Get(0)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_1_info = udpApp.Get(1)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();
        ASSERT_EQUAL(outgoing_0_info.size(), 1);
        ASSERT_EQUAL(std::get<0>(outgoing_0_info.at(0)).GetUdpBurstId(), 0);
        ASSERT_EQUAL_APPROX((double) std::get<1>(outgoing_0_info.at(0)), 0.7 * 15 * 1000 * 1000 / 8.0 / 1500.0, 0.00001); // Everything will be sent
        ASSERT_EQUAL(outgoing_1_info.size(), 0);
        ASSERT_EQUAL(incoming_0_info.size(), 0);
        ASSERT_EQUAL(incoming_1_info.size(), 1);
        ASSERT_EQUAL(std::get<0>(incoming_1_info.at(0)).GetUdpBurstId(), 0);
        ASSERT_EQUAL_APPROX((double) std::get<1>(incoming_1_info.at(0)), 0.7 * 15 * 1000 * 1000 / 8.0 / 1500.0, 100.0); // Not everything will arrive due to TCP competition

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_0_cwnd.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_0_progress.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_0_rtt.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_1_cwnd.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_1_progress.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flow_1_rtt.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_burst_0_outgoing.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_burst_0_incoming.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////
