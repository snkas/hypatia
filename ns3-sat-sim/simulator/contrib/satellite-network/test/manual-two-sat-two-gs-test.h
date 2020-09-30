/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <stdexcept>

#include "ns3/basic-simulation.h"
#include "ns3/udp-burst-scheduler.h"
#include "ns3/topology-satellite-network.h"
#include "ns3/tcp-optimizer.h"
#include "ns3/arbiter-single-forward-helper.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/gsl-if-bandwidth-helper.h"

#include "ns3/test.h"
#include "test-helpers.h"

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////


class ArbiterCustom: public Arbiter
{
public:

    ArbiterCustom(Ptr<Node> this_node, NodeContainer nodes) : Arbiter(this_node, nodes) {
        // Left empty intentionally
    }

    ArbiterResult Decide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) {

        // std::cout << Simulator::Now() << " at " << m_node_id << std::endl;

        // Retrieve the components
        int32_t next_node_id;
        int32_t own_if_id;
        int32_t next_if_id;
        if (m_node_id == 0) {
            if (target_node_id == 0) {
                throw std::runtime_error("To itself.");
            } else if (target_node_id == 1) {
                next_node_id = 1;
                own_if_id = 1;
                next_if_id = 1;
            } else if (target_node_id == 2) {
                next_node_id = 2;
                own_if_id = 2;
                next_if_id = 1;
            } else if (target_node_id == 3) {
                next_node_id = 1;
                own_if_id = 1;
                next_if_id = 1;
            } else {
                throw std::runtime_error("Node target does not exist.");
            }
        } else if (m_node_id == 1) {
            if (target_node_id == 0) {
                next_node_id = 0;
                own_if_id = 1;
                next_if_id = 1;
            } else if (target_node_id == 1) {
                throw std::runtime_error("To itself.");
            } else if (target_node_id == 2) {
                next_node_id = 0;
                own_if_id = 1;
                next_if_id = 1;
            } else if (target_node_id == 3) {
                next_node_id = 3;
                own_if_id = 2;
                next_if_id = 1;
            } else {
                throw std::runtime_error("Node target does not exist.");
            }
        } else if (m_node_id == 2) {
            if (target_node_id == 0) {
                next_node_id = 0;
                own_if_id = 1;
                next_if_id = 2;
            } else if (target_node_id == 1) {
                next_node_id = 0;
                own_if_id = 1;
                next_if_id = 2;
            } else if (target_node_id == 2) {
                throw std::runtime_error("To itself.");
            } else if (target_node_id == 3) {
                next_node_id = 0;
                own_if_id = 1;
                next_if_id = 2;
            } else {
                throw std::runtime_error("Node target does not exist.");
            }
        } else if (m_node_id == 3) {
            if (target_node_id == 0) {
                next_node_id = 1;
                own_if_id = 1;
                next_if_id = 2;
            } else if (target_node_id == 1) {
                next_node_id = 1;
                own_if_id = 1;
                next_if_id = 2;
            } else if (target_node_id == 2) {
                next_node_id = 1;
                own_if_id = 1;
                next_if_id = 2;
            } else if (target_node_id == 3) {
                throw std::runtime_error("To itself.");
            } else {
                throw std::runtime_error("Node target does not exist.");
            }
        } else {
            throw std::runtime_error("Node does not exist.");
        }

        // Invalid selected node id
        if (next_node_id != -1) {

            // Retrieve the IP gateway
            uint32_t select_ip_gateway = m_nodes.Get(next_node_id)->GetObject<Ipv4>()->GetAddress(next_if_id, 0).GetLocal().Get();

            // We succeeded in finding the interface and gateway to the next hop
            return ArbiterResult(false, own_if_id, select_ip_gateway);

        } else {
            return ArbiterResult(true, 0, 0); // Failed = no route (means either drop, or socket fails)
        }

    }

    std::string StringReprOfForwardingState() {
        return "This is a test -- not implemented";
    }

};

class ManualTwoSatTwoGsTest : public TestCase {
public:
    ManualTwoSatTwoGsTest () : TestCase ("manual-two-sat-two-gs") {};

    void DoRun () {

        const std::string temp_dir = ".tmp-manual-two-sat-two-gs-test";

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);

        // A configuration file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1000000000" << std::endl; // 1s
        config_file << "simulation_seed=987654321" << std::endl;
        config_file.close();

        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);

        // Satellites have node id 0 and 1
        // Ground stations have node id 2 and 3

        // Layout:
        //
        // Satellites:       0 -- <ISL> -- 1
        //
        //                       <GSLs>
        //
        // Ground stations:  2             3

        // Containers
        NodeContainer allNodes;                           //!< All nodes
        NodeContainer groundStationNodes;                 //!< Ground station nodes
        NodeContainer satelliteNodes;                     //!< Satellite nodes

        // Create the nodes
        satelliteNodes.Create(2);
        groundStationNodes.Create(2);
        allNodes.Add(satelliteNodes);
        allNodes.Add(groundStationNodes);

        //////////////////////
        // Mobility models

        // Mobility for satellite nodes
        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        
        // Satellites mobility models
        mobility.Install(satelliteNodes.Get(0));
        satelliteNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(-1000000, -1000000, -1000000));
        mobility.Install(satelliteNodes.Get(1));
        satelliteNodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(1000000, -1000000, -1000000));

        // Ground stations mobility models
        mobility.Install(groundStationNodes.Get(0));
        groundStationNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(1000000, 1000000, -1000000));
        mobility.Install(groundStationNodes.Get(1));
        groundStationNodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(1000000, -1000000, 1000000));

        //////////////////////
        // IPv4 stack with routing arbiter

        Ipv4ArbiterRoutingHelper ipv4RoutingHelper = Ipv4ArbiterRoutingHelper();
        InternetStackHelper internet;
        internet.SetRoutingHelper(ipv4RoutingHelper);
        internet.Install(allNodes);

        // Helper for IP address assignment later on
        Ipv4AddressHelper ipv4_helper;
        ipv4_helper.SetBase ("10.0.0.0", "255.255.255.0");
        
        //////////////////////
        // ISLs

        // One ISL
        PointToPointLaserHelper p2p_laser_helper;
        p2p_laser_helper.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue(QueueSize("100p")));
        p2p_laser_helper.SetDeviceAttribute ("DataRate", DataRateValue(DataRate("4Mbps")));

        // Traffic control helper
        TrafficControlHelper tch_isl;
        tch_isl.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", QueueSizeValue(QueueSize("1p"))); // Will be removed later any case

        // Install a p2p laser link between these two satellites
        NodeContainer c;
        c.Add(satelliteNodes.Get(0));
        c.Add(satelliteNodes.Get(1));
        NetDeviceContainer netDevices = p2p_laser_helper.Install(c);

        // Install traffic control helper
        tch_isl.Install(netDevices.Get(0));
        tch_isl.Install(netDevices.Get(1));

        // Assign some IP address (nothing smart, no aggregation, just some IP address)
        ipv4_helper.Assign(netDevices);
        ipv4_helper.NewNetwork();

        // Remove the traffic control layer (must be done here, else the Ipv4 helper will assign a default one)
        TrafficControlHelper tch_uninstaller;
        tch_uninstaller.Uninstall(netDevices.Get(0));
        tch_uninstaller.Uninstall(netDevices.Get(1));

        //////////////////////
        // GSLs

        // Link helper
        GSLHelper gsl_helper;;
        gsl_helper.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue(QueueSize("100p")));
        gsl_helper.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("7Mbps")));

        // Traffic control helper
        TrafficControlHelper tch_gsl;
        tch_gsl.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize", QueueSizeValue(QueueSize("1p")));  // Will be removed later any case

        std::vector<std::tuple<int32_t, double>> node_gsl_if_info;
        node_gsl_if_info.push_back(std::make_tuple(1, 1.0));
        node_gsl_if_info.push_back(std::make_tuple(1, 1.0));
        node_gsl_if_info.push_back(std::make_tuple(1, 1.0));
        node_gsl_if_info.push_back(std::make_tuple(1, 1.0));

        // Create and install GSL network devices
        NetDeviceContainer devices = gsl_helper.Install(
                satelliteNodes,
                groundStationNodes,
                node_gsl_if_info
        );

        // Install queueing disciplines
        tch_gsl.Install(devices);

        // Assign IP addresses
        for (uint32_t i = 0; i < devices.GetN(); i++) {
            ipv4_helper.Assign(devices.Get(i));
            ipv4_helper.NewNetwork();
        }

        // Remove the traffic control layer (must be done here, else the Ipv4 helper will assign a default one)
        TrafficControlHelper tch_gsl_uninstaller;
        tch_gsl_uninstaller.Uninstall(devices);

        //////////////////////
        // ARP lookup table filling

        // ARP lookups hinder performance, and actually won't succeed, so to prevent that from happening,
        // all GSL interfaces' IPs are added into an ARP cache

        // ARP cache with all ground station and satellite GSL channel interface info
        Ptr<ArpCache> arpAll = CreateObject<ArpCache>();
        arpAll->SetAliveTimeout (Seconds(3600 * 24 * 365)); // Valid one year

        // Satellite ARP entries
        for (uint32_t i = 0; i < allNodes.GetN(); i++) {

            // Information about all interfaces (TODO: Only needs to be GSL interfaces)
            for (size_t j = 1; j < allNodes.Get(i)->GetObject<Ipv4>()->GetNInterfaces(); j++) {
                Mac48Address mac48Address = Mac48Address::ConvertFrom(allNodes.Get(i)->GetObject<Ipv4>()->GetNetDevice(j)->GetAddress());
                Ipv4Address ipv4Address = allNodes.Get(i)->GetObject<Ipv4>()->GetAddress(j, 0).GetLocal();

                // Add the info of the GSL interface to the cache
                ArpCache::Entry * entry = arpAll->Add(ipv4Address);
                entry->SetMacAddress(mac48Address);

                // Set a pointer to the ARP cache it should use (will be filled at the end of this function, it's only a pointer)
                allNodes.Get(i)->GetObject<Ipv4L3Protocol>()->GetInterface(j)->SetAttribute("ArpCache", PointerValue(arpAll));

            }

        }

        //////////////////////nodes
        // Arbiter routing

        Ptr<ArbiterCustom> arbiter;

        arbiter = CreateObject<ArbiterCustom>(allNodes.Get(0), allNodes);
        allNodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom>(allNodes.Get(1), allNodes);
        allNodes.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom>(allNodes.Get(2), allNodes);
        allNodes.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom>(allNodes.Get(3), allNodes);
        allNodes.Get(3)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        //////////////////////
        // UDP application

        // Install a UDP burst client on all
        UdpBurstHelper udpBurstHelper(1026, basicSimulation->GetLogsDir());
        ApplicationContainer udpApp = udpBurstHelper.Install(allNodes);
        udpApp.Start(Seconds(0.0));

        // UDP burst info entry
        UdpBurstInfo udpBurstInfo(
                0,
                2,
                3,
                3, // Rate in Mbit/s
                0,
                100000000000, // Duration in ns // 100000000000
                "abc",
                "def"
        );

        // Register all bursts being sent from there and being received
        udpApp.Get(2)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                udpBurstInfo,
                InetSocketAddress(allNodes.Get(3)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                true
        );
        udpApp.Get(3)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                udpBurstInfo,
                true
        );

        // Run simulation
        basicSimulation->Run();

        // Check UDP burst completion information
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> outgoing_2_info = udpApp.Get(2)->GetObject<UdpBurstApplication>()->GetOutgoingBurstsInformation();
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> outgoing_3_info = udpApp.Get(3)->GetObject<UdpBurstApplication>()->GetOutgoingBurstsInformation();
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_2_info = udpApp.Get(2)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_3_info = udpApp.Get(3)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();

        // Node 2 sends out
        ASSERT_EQUAL(outgoing_2_info.size(), 1);
        ASSERT_EQUAL(std::get<0>(outgoing_2_info.at(0)).GetUdpBurstId(), 0);
        double expected_sent = 3.0 * 1000 * 1000 / 8.0 / 1500.0;
        ASSERT_EQUAL_APPROX((double) std::get<1>(outgoing_2_info.at(0)), expected_sent, 0.00001);

        // Node 2 does not receive
        ASSERT_EQUAL(incoming_2_info.size(), 0);

        // Node 3 does not send
        ASSERT_EQUAL(outgoing_3_info.size(), 0);

        // Node 3 does receive
        ASSERT_EQUAL(incoming_3_info.size(), 1);
        ASSERT_EQUAL(std::get<0>(incoming_3_info.at(0)).GetUdpBurstId(), 0);
        double expected_received = 3.0 * 1000 * 1000 / 8.0 / 1500.0;
        ASSERT_EQUAL_APPROX((double) std::get<1>(incoming_3_info.at(0)), expected_received, 10);

        // Check the RTTs

        // Outgoing
        std::vector<std::string> lines_precise_outgoing_csv = read_file_direct(temp_dir + "/logs_ns3/udp_burst_0_outgoing.csv");
        ASSERT_EQUAL_APPROX(lines_precise_outgoing_csv.size(), expected_sent, 0.00001);
        int j = 0;
        std::vector<int64_t> sent_timestamps;
        for (std::string line : lines_precise_outgoing_csv) {
            std::vector <std::string> line_spl = split_string(line, ",");
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), j * std::ceil(1500.0 / (3.0 / 8000.0)));
            sent_timestamps.push_back(parse_positive_int64(line_spl[2]));
            j += 1;
        }

        // Incoming
        std::vector<std::string> lines_precise_incoming_csv = read_file_direct(temp_dir + "/logs_ns3/udp_burst_0_incoming.csv");
        ASSERT_EQUAL_APPROX(lines_precise_incoming_csv.size(), std::get<1>(incoming_3_info.at(0)), 0.00001);
        j = 0;
        for (std::string line : lines_precise_incoming_csv) {
            std::vector <std::string> line_spl = split_string(line, ",");
            double hop_a_distance_m = std::sqrt(std::pow(1000000 - (-1000000), 2) + std::pow(1000000 - (-1000000), 2) + std::pow(-1000000 - (-1000000), 2));
            double hop_a_latency_ns = hop_a_distance_m / (299792458.0 / 1000000000.0);

            double hop_b_distance_m = std::sqrt(std::pow(-1000000 - (1000000), 2) + std::pow(-1000000 - (-1000000), 2) + std::pow(-1000000 - (-1000000), 2));
            double hop_b_latency_ns = hop_b_distance_m / (299792458.0 / 1000000000.0);

            double hop_c_distance_m = std::sqrt(std::pow(1000000 - (1000000), 2) + std::pow(-1000000 - (-1000000), 2) + std::pow(-1000000 - (1000000), 2));
            double hop_c_latency_ns = hop_c_distance_m / (299792458.0 / 1000000000.0);

            double time_one_way_latency_ns = hop_a_latency_ns + hop_b_latency_ns + hop_c_latency_ns;

            // std::cout << " Hop A: " << hop_a_distance_m << " m gives latency " << hop_a_latency_ns << " ns" << std::endl;
            // std::cout << " Hop B: " << hop_b_distance_m << " m gives latency " << hop_b_latency_ns << " ns" << std::endl;
            // std::cout << " Hop C: " << hop_c_distance_m << " m gives latency " << hop_c_latency_ns << " ns" << std::endl;

            // At most 10 nanoseconds off due to rounding on the way
            ASSERT_EQUAL_APPROX(
                    parse_positive_int64(line_spl[2]) - sent_timestamps[j],
                    time_one_way_latency_ns + (1502 / (0.000125 * 7.0)) + (1502.0 / (0.000125 * 4.0)) + (1502.0 / (0.000125 * 7.0)),
                    10
            );

            j += 1;
        }

        // Finalize the simulation
        basicSimulation->Finalize();

    }

};

////////////////////////////////////////////////////////////////////////////////////////
