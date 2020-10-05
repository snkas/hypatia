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
#include "ns3/tcp-flow-scheduler.h"
#include "ns3/gsl-channel.h"
#include "ns3/point-to-point-laser-channel.h"

#include "ns3/test.h"
#include "test-helpers.h"

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class ManualTwoSatTwoGsTest : public TestCase {
public:

    NodeContainer allNodes; //!< All nodes

    ManualTwoSatTwoGsTest(std::string s) : TestCase(s) {};

    void setup_scenario(double distance_multiplier, bool new_prop_speed, double new_prop_speed_m_per_s) {

        // Clear all nodes
        allNodes = NodeContainer();

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
        satelliteNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(-1 * distance_multiplier, -1 * distance_multiplier, -1 * distance_multiplier));
        mobility.Install(satelliteNodes.Get(1));
        satelliteNodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(1 * distance_multiplier, -1 * distance_multiplier, -1 * distance_multiplier));

        // Ground stations mobility models
        mobility.Install(groundStationNodes.Get(0));
        groundStationNodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(1 * distance_multiplier, 1 * distance_multiplier, -1 * distance_multiplier));
        mobility.Install(groundStationNodes.Get(1));
        groundStationNodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(1 * distance_multiplier, -1 * distance_multiplier, 1 * distance_multiplier));

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
        // Checks about what was installed until now

        // The ISL devices
        for (uint32_t i = 0; i < netDevices.GetN(); i++) {
            Ptr<PointToPointLaserNetDevice> islNetDevice = netDevices.Get(i)->GetObject<PointToPointLaserNetDevice>();
            ASSERT_TRUE(islNetDevice->IsBroadcast());
            islNetDevice->GetBroadcast();
            ASSERT_TRUE(islNetDevice->IsMulticast());
            islNetDevice->GetMulticast(Ipv4Address());
            islNetDevice->GetMulticast(Ipv6Address());
            ASSERT_TRUE(islNetDevice->IsPointToPoint());
            ASSERT_FALSE(islNetDevice->IsBridge());
            ASSERT_FALSE(islNetDevice->SupportsSendFrom());
            ASSERT_EQUAL(islNetDevice->GetIfIndex(), 1);
            ASSERT_EQUAL(islNetDevice->GetChannel()->GetNDevices(), 2);
            if (i == 0) {
                ASSERT_EQUAL(islNetDevice->GetDestinationNode()->GetId(), 1);
            } else {
                ASSERT_EQUAL(islNetDevice->GetDestinationNode()->GetId(), 0);
            }
            Ptr<DropTailQueue<Packet>> queue = islNetDevice->GetQueue()->GetObject<DropTailQueue<Packet>>();
            QueueSize qs = queue->GetMaxSize();
            ASSERT_EQUAL(qs.GetUnit(), ns3::PACKETS);
            ASSERT_EQUAL(qs.GetValue(), 100);
        }

        //////////////////////
        // GSLs

        // Link helper
        GSLHelper gsl_helper;
        if (new_prop_speed) {
            gsl_helper.SetChannelAttribute ( "PropagationSpeed", DoubleValue(new_prop_speed_m_per_s));
        }
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
        // Checks about what was installed until now

        // The GSL devices
        for (uint32_t i = 0; i < devices.GetN(); i++) {
            Ptr<GSLNetDevice> gslNetDevice = devices.Get(i)->GetObject<GSLNetDevice>();
            ASSERT_TRUE(gslNetDevice->IsBroadcast());
            ASSERT_EXCEPTION(gslNetDevice->GetBroadcast());
            ASSERT_FALSE(gslNetDevice->IsMulticast());
            ASSERT_EXCEPTION(gslNetDevice->GetMulticast(Ipv4Address()));
            ASSERT_EXCEPTION(gslNetDevice->GetMulticast(Ipv6Address()));
            ASSERT_FALSE(gslNetDevice->IsPointToPoint());
            ASSERT_FALSE(gslNetDevice->IsBridge());
            ASSERT_FALSE(gslNetDevice->SupportsSendFrom());
            if (i == 0) {
                ASSERT_EQUAL(gslNetDevice->GetIfIndex(), 2);
            } else if (i == 1) {
                ASSERT_EQUAL(gslNetDevice->GetIfIndex(), 2);
            } else if (i == 2) {
                ASSERT_EQUAL(gslNetDevice->GetIfIndex(), 1);
            } else if (i == 3) {
                ASSERT_EQUAL(gslNetDevice->GetIfIndex(), 1);
            }
            Ptr<DropTailQueue<Packet>> queue = gslNetDevice->GetQueue()->GetObject<DropTailQueue<Packet>>();
            QueueSize qs = queue->GetMaxSize();
            ASSERT_EQUAL(qs.GetUnit(), ns3::PACKETS);
            ASSERT_EQUAL(qs.GetValue(), 100);
        }

        // Some small checks about what was installed
        ASSERT_EQUAL(4, allNodes.Get(2)->GetObject<Ipv4>()->GetNetDevice(1)->GetChannel()->GetObject<GSLChannel>()->GetNDevices());
        ASSERT_EQUAL(devices.Get(1), allNodes.Get(2)->GetObject<Ipv4>()->GetNetDevice(1)->GetChannel()->GetObject<GSLChannel>()->GetDevice(1));

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

    }

};

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

////////////////////////////////////////////////////////////////////////////////////////

class ManualTwoSatTwoGsFirstTest : public ManualTwoSatTwoGsTest {
public:
    ManualTwoSatTwoGsFirstTest () : ManualTwoSatTwoGsTest ("manual-two-sat-two-gs first") {};

    void DoRun () {

        const std::string temp_dir = ".tmp-manual-two-sat-two-gs-first-test";

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

        // Install the scenario
        setup_scenario(1000000.0, false, 0.0);

        //////////////////////
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

class ManualTwoSatTwoGsDifferentPropSpeedTest : public ManualTwoSatTwoGsTest {
public:
    ManualTwoSatTwoGsDifferentPropSpeedTest () : ManualTwoSatTwoGsTest ("manual-two-sat-two-gs different-prop-speed") {};

    void DoRun () {

        const std::string temp_dir = ".tmp-manual-two-sat-two-gs-different-prop-speed-test";

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

        // Install the scenario
        setup_scenario(1000000.0, true, 100000711.0);

        //////////////////////
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
        ASSERT_EQUAL_APPROX((double) std::get<1>(incoming_3_info.at(0)), expected_received, 20);

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
            double hop_a_latency_ns = hop_a_distance_m / (100000711.0 / 1000000000.0);

            double hop_b_distance_m = std::sqrt(std::pow(-1000000 - (1000000), 2) + std::pow(-1000000 - (-1000000), 2) + std::pow(-1000000 - (-1000000), 2));
            double hop_b_latency_ns = hop_b_distance_m / (299792458.0 / 1000000000.0);

            double hop_c_distance_m = std::sqrt(std::pow(1000000 - (1000000), 2) + std::pow(-1000000 - (-1000000), 2) + std::pow(-1000000 - (1000000), 2));
            double hop_c_latency_ns = hop_c_distance_m / (100000711.0 / 1000000000.0);

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

class ArbiterCustom2: public Arbiter
{
public:

    ArbiterCustom2(Ptr<Node> this_node, NodeContainer nodes) : Arbiter(this_node, nodes) {
        // Left empty intentionally
    }

    ArbiterResult Decide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) {

//        if (source_node_id == 3) {
//            std::cout << Simulator::Now() << " at " << m_node_id << std::endl;
//        }

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
                next_node_id = 3;
                own_if_id = 2;
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
                next_node_id = 2;
                own_if_id = 2;
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
                next_node_id = 1;
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
                next_node_id = 0;
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

////////////////////////////////////////////////////////////////////////////////////////

class ManualTwoSatTwoGsUpSharedTest : public ManualTwoSatTwoGsTest {
public:
    ManualTwoSatTwoGsUpSharedTest () : ManualTwoSatTwoGsTest ("manual-two-sat-two-gs up-shared") {};

    void DoRun () {

        const std::string temp_dir = ".tmp-manual-two-sat-two-gs-up-shared-test";

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);

        // A configuration file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=2000000000" << std::endl; // 1s
        config_file << "simulation_seed=987654321" << std::endl;
        config_file.close();

        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);

        // Basic optimization
        TcpOptimizer::OptimizeBasic(basicSimulation);

        // Install the scenario
        setup_scenario(1000000.0, false, 0.0);

        //////////////////////
        // Arbiter routing

        Ptr<ArbiterCustom2> arbiter;

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(0), allNodes);
        allNodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(1), allNodes);
        allNodes.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(2), allNodes);
        allNodes.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(3), allNodes);
        allNodes.Get(3)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        //////////////////////
        // UDP application

        // Install a UDP burst client on all
        UdpBurstHelper udpBurstHelper(1026, basicSimulation->GetLogsDir());
        ApplicationContainer udpApp = udpBurstHelper.Install(allNodes);
        udpApp.Start(Seconds(0.0));

        // UDP burst info entry
        int src_udp_id = 3;
        int dst_udp_id = 2;
        UdpBurstInfo udpBurstInfo(
                0,
                src_udp_id,
                dst_udp_id,
                2, // Rate in Mbit/s
                0,
                100000000000, // Duration in ns // 100000000000
                "abc",
                "def"
        );

        // Register all bursts being sent from there and being received
        udpApp.Get(src_udp_id)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                udpBurstInfo,
                InetSocketAddress(allNodes.Get(dst_udp_id)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                true
        );
        udpApp.Get(dst_udp_id)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                udpBurstInfo,
                true
        );

        //////////////////////
        // TCP application

        // Install flow sink on all
        TcpFlowSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 1024));
        ApplicationContainer app = sink.Install(allNodes);
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(10000000000000));

        // 3 --> 0
        int src_tcp_id = 3;
        int dst_tcp_id = 0;
        TcpFlowSendHelper source0(
                "ns3::TcpSocketFactory",
                InetSocketAddress(allNodes.Get(dst_tcp_id)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                100000000,
                0,
                true,
                basicSimulation->GetLogsDir(),
                ""
        );
        app = source0.Install(allNodes.Get(src_tcp_id));
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(10000000000));

        // Run simulation
        basicSimulation->Run();

        // TCP flow should have about 5 Mbit/s
        std::cout << "TCP rate: " << app.Get(0)->GetObject<TcpFlowSendApplication>()->GetAckedBytes() / 2.0 / 125000.0 << std::endl;

        // And the UDP flow around 2 Mbit/s
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_3_info = udpApp.Get(dst_udp_id)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();
        std::cout << "UDP rate: " << (double) std::get<1>(incoming_3_info.at(0)) * 1500.0 / 2.0 / 125000.0 << std::endl;

        // And the tests
        ASSERT_EQUAL_APPROX(app.Get(0)->GetObject<TcpFlowSendApplication>()->GetAckedBytes() / 2.0 / 125000.0, 5.0, 1.0);
        ASSERT_EQUAL_APPROX((double) std::get<1>(incoming_3_info.at(0)) * 1500.0 / 2.0 / 125000.0, 2.0, 0.1);

        // Finalize the simulation
        basicSimulation->Finalize();

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class ManualTwoSatTwoGsUpSharedUdpTest : public ManualTwoSatTwoGsTest {
public:
    ManualTwoSatTwoGsUpSharedUdpTest () : ManualTwoSatTwoGsTest ("manual-two-sat-two-gs up-shared-udp") {};

    void DoRun () {

        // Test many configs
        std::vector<std::tuple<int64_t, int64_t, double, double, int64_t, int64_t, double, double>> test_configs;

        // Ground station sends to other ground station and to one satellite
        test_configs.push_back(std::make_tuple(3, 2, 4.0, 3.5, 3, 0, 4.0, 3.5));
        test_configs.push_back(std::make_tuple(3, 2, 4.0, 3.5, 3, 2, 4.0, 3.5));

        // Ground station sends two flows to one satellite
        test_configs.push_back(std::make_tuple(3, 1, 4.0, 3.5, 3, 1, 4.0, 3.5));
        test_configs.push_back(std::make_tuple(3, 0, 4.0, 3.5, 3, 0, 4.0, 3.5));

        // Satellite sends to other satellite and one ground station
        test_configs.push_back(std::make_tuple(0, 1, 20.0, 4.0, 0, 2, 20.0, 7.0));
        test_configs.push_back(std::make_tuple(0, 1, 20.0, 4.0, 0, 3, 20.0, 7.0));
        test_configs.push_back(std::make_tuple(1, 0, 20.0, 4.0, 1, 2, 20.0, 7.0));
        test_configs.push_back(std::make_tuple(1, 0, 20.0, 4.0, 1, 3, 20.0, 7.0));

        // Over the ISL
        test_configs.push_back(std::make_tuple(0, 1, 3.0, 2.0, 0, 1, 3.0, 2.0));
        test_configs.push_back(std::make_tuple(1, 0, 3.0, 2.0, 1, 0, 3.0, 2.0));
        test_configs.push_back(std::make_tuple(0, 1, 20.0, 4.0, 1, 0, 20.0, 4.0));
        test_configs.push_back(std::make_tuple(1, 0, 20.0, 4.0, 0, 1, 20.0, 4.0));

        // Both satellite send full down to one ground station
        test_configs.push_back(std::make_tuple(1, 2, 20.0, 7.0, 0, 2, 20.0, 7.0));
        test_configs.push_back(std::make_tuple(1, 3, 20.0, 7.0, 0, 3, 20.0, 7.0));

        // Both ground stations send full to one satellite each
        test_configs.push_back(std::make_tuple(3, 0, 20.0, 7.0, 2, 0, 20.0, 7.0));
        test_configs.push_back(std::make_tuple(3, 1, 20.0, 7.0, 2, 1, 20.0, 7.0));

        // Each ground station sends to one satellite
        test_configs.push_back(std::make_tuple(3, 0, 20.0, 7.0, 2, 1, 20.0, 7.0));
        test_configs.push_back(std::make_tuple(3, 1, 20.0, 7.0, 2, 0, 20.0, 7.0));

        // Check outcome of each config
        for (size_t i = 0; i < test_configs.size(); i++) {
            std::cout << "UDP config: " << i << std::endl;
            std::tuple<int64_t, int64_t, double, double, int64_t, int64_t, double, double> config = test_configs.at(i);

            // Retrieve from config
            int src_udp_id_1 = std::get<0>(config);
            int dst_udp_id_1 = std::get<1>(config);
            double burst_1_rate = std::get<2>(config);
            double burst_1_exp_rate = std::get<3>(config);
            int src_udp_id_2 = std::get<4>(config);
            int dst_udp_id_2 = std::get<5>(config);
            double burst_2_rate = std::get<6>(config);
            double burst_2_exp_rate = std::get<7>(config);

            const std::string temp_dir = ".tmp-manual-two-sat-two-gs-up-shared-udp-test";

            // Create temporary run directory
            mkdir_if_not_exists(temp_dir);

            // A configuration file
            std::ofstream config_file;
            config_file.open (temp_dir + "/config_ns3.properties");
            int64_t duration_ns = 500000000;
            double duration_s = duration_ns / 1e9;
            config_file << "simulation_end_time_ns=" << duration_ns << std::endl;
            config_file << "simulation_seed=987654321" << std::endl;
            config_file.close();

            // Load basic simulation environment
            Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);

            // Basic optimization
            TcpOptimizer::OptimizeBasic(basicSimulation);

            // Install the scenario
            setup_scenario(1000000.0, false, 0.0);

            //////////////////////
            // Arbiter routing

            Ptr<ArbiterCustom2> arbiter;

            arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(0), allNodes);
            allNodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

            arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(1), allNodes);
            allNodes.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

            arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(2), allNodes);
            allNodes.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

            arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(3), allNodes);
            allNodes.Get(3)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

            //////////////////////
            // UDP application

            // Install a UDP burst client on all
            UdpBurstHelper udpBurstHelper(1026, basicSimulation->GetLogsDir());
            ApplicationContainer udpApp = udpBurstHelper.Install(allNodes);
            udpApp.Start(Seconds(0.0));

            // UDP burst info entry
            UdpBurstInfo udpBurstInfo1(
                    0,
                    src_udp_id_1,
                    dst_udp_id_1,
                    burst_1_rate, // Rate in Mbit/s
                    0,
                    100000000000, // Duration in ns // 100000000000
                    "abc",
                    "def"
            );
            udpApp.Get(src_udp_id_1)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                    udpBurstInfo1,
                    InetSocketAddress(allNodes.Get(dst_udp_id_1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                    true
            );
            udpApp.Get(dst_udp_id_1)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                    udpBurstInfo1,
                    true
            );

            // UDP burst info entry
            UdpBurstInfo udpBurstInfo2(
                    1,
                    src_udp_id_2,
                    dst_udp_id_2,
                    burst_2_rate, // Rate in Mbit/s
                    0,
                    100000000000, // Duration in ns // 100000000000
                    "abc",
                    "def"
            );
            udpApp.Get(src_udp_id_2)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                    udpBurstInfo2,
                    InetSocketAddress(allNodes.Get(dst_udp_id_2)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                    true
            );
            udpApp.Get(dst_udp_id_2)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                    udpBurstInfo2,
                    true
            );

            // Run simulation
            basicSimulation->Run();

            // And the UDP flows should have half of the 7 Mbit/s up of the ground station each
            std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_1_info = udpApp.Get(dst_udp_id_1)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();
            std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_2_info = udpApp.Get(dst_udp_id_2)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();
            size_t index_2 = 0;
            if (dst_udp_id_1 == dst_udp_id_2) {
                index_2 = 1;
            }
            double measured_rate_1 = (double) std::get<1>(incoming_1_info.at(0)) * 1500.0 / duration_s / 125000.0;
            double measured_rate_2 = (double) std::get<1>(incoming_2_info.at(index_2)) * 1500.0 / duration_s / 125000.0;
            std::cout << "UDP Rate 1: " << measured_rate_1 << std::endl;
            std::cout << "UDP Rate 2: " << measured_rate_2 << std::endl;

            // Checks
            ASSERT_EQUAL_APPROX(measured_rate_1, burst_1_exp_rate, 0.2);
            ASSERT_EQUAL_APPROX(measured_rate_2, burst_2_exp_rate, 0.2);
            ASSERT_TRUE(measured_rate_1 + measured_rate_2 <= burst_1_exp_rate + burst_2_exp_rate);

            // Finalize the simulation
            basicSimulation->Finalize();

        }

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class ManualTwoSatTwoGsDownBothFullTest : public ManualTwoSatTwoGsTest {
public:
    ManualTwoSatTwoGsDownBothFullTest () : ManualTwoSatTwoGsTest ("manual-two-sat-two-gs down-both-full") {};

    void DoRun () {

        const std::string temp_dir = ".tmp-manual-two-sat-two-gs-down-both-full-test";

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);

        // A configuration file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=2000000000" << std::endl; // 2s
        config_file << "simulation_seed=987654321" << std::endl;
        config_file.close();

        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);

        // Basic optimization
        TcpOptimizer::OptimizeBasic(basicSimulation);

        // Install the scenario
        setup_scenario(1000000.0, false, 0.0);

        //////////////////////
        // Arbiter routing

        Ptr<ArbiterCustom2> arbiter;

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(0), allNodes);
        allNodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(1), allNodes);
        allNodes.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(2), allNodes);
        allNodes.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        arbiter = CreateObject<ArbiterCustom2>(allNodes.Get(3), allNodes);
        allNodes.Get(3)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);

        // 3 to 1 has 6.6712819 ms propagation delay
        // Need 2 * 6.6712819 ms * 7 Mbit/s / 1500 byte ~= 8 packets to sustain full line rate

        //////////////////////
        // UDP application

        // Install a UDP burst client on all
        UdpBurstHelper udpBurstHelper(1026, basicSimulation->GetLogsDir());
        ApplicationContainer udpApp = udpBurstHelper.Install(allNodes);
        udpApp.Start(Seconds(0.0));

        // UDP burst info entry
        int src_udp_id = 0;
        int dst_udp_id = 2;
        UdpBurstInfo udpBurstInfo(
                0,
                src_udp_id,
                dst_udp_id,
                6.0, // Rate in Mbit/s
                0,
                100000000000, // Duration in ns // 100000000000
                "abc",
                "def"
        );
        udpApp.Get(src_udp_id)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                udpBurstInfo,
                InetSocketAddress(allNodes.Get(dst_udp_id)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                true
        );
        udpApp.Get(dst_udp_id)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                udpBurstInfo,
                true
        );

        //////////////////////
        // TCP application

        // Install flow sink on all
        TcpFlowSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 1024));
        ApplicationContainer app = sink.Install(allNodes);
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(10000000000000));

        // src --> dst
        int src_tcp_id = 3;
        int dst_tcp_id = 2;
        TcpFlowSendHelper source0(
                "ns3::TcpSocketFactory",
                InetSocketAddress(allNodes.Get(dst_tcp_id)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                10000000000,
                0,
                true,
                basicSimulation->GetLogsDir(),
                ""
        );
        app = source0.Install(allNodes.Get(src_tcp_id));
        app.Start(NanoSeconds(0));
        app.Stop(NanoSeconds(10000000000));

        // Run simulation
        basicSimulation->Run();

        // UDP info
        std::vector<std::tuple<UdpBurstInfo, uint64_t>> incoming_udp_info = udpApp.Get(dst_udp_id)->GetObject<UdpBurstApplication>()->GetIncomingBurstsInformation();

//        std::cout << "TCP Rate:" << app.Get(0)->GetObject<TcpFlowSendApplication>()->GetAckedBytes() / 2.0 / 125000.0 << std::endl;
//        std::cout << "UDP Rate:" << (double) std::get<1>(incoming_2_info.at(0)) * 1500.0 / 1.0 / 125000.0  << std::endl;

        // TCP flow should have about 5.5-7 Mbit/s (TCP is not great)
        ASSERT_EQUAL_APPROX(app.Get(0)->GetObject<TcpFlowSendApplication>()->GetAckedBytes() / 2.0 / 125000.0, 7.0, 1.5);

        // And the UDP flow around 6 Mbit/s
        ASSERT_EQUAL_APPROX((double) std::get<1>(incoming_udp_info.at(0)) * 1500.0 / 2.0 / 125000.0, 6, 0.2);

        // Finalize the simulation
        basicSimulation->Finalize();

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class ManualTwoSatTwoGsChangingForwardingTest : public ManualTwoSatTwoGsTest {
public:
    ManualTwoSatTwoGsChangingForwardingTest () : ManualTwoSatTwoGsTest ("manual-two-sat-two-gs changing-forwarding") {};

    void DoRun () {

        // Retrieve from config
        int src_udp_id_1 = 2;
        int dst_udp_id_1 = 3;
        double burst_1_rate = 100.0;

        const std::string temp_dir = ".tmp-manual-two-sat-two-gs-changing-forwarding-test";

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);
        mkdir_if_not_exists(temp_dir + "/network_state");

        // Configuration file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=4000000000" << std::endl; // 4s duration
        config_file << "simulation_seed=987654321" << std::endl;
        config_file << "dynamic_state_update_interval_ns=1000000000" << std::endl; // Every 1000ms
        config_file << "satellite_network_routes_dir=network_state" << std::endl;
        config_file << "satellite_network_force_static=false" << std::endl;
        config_file.close();

        // Forwarding state files
        std::ofstream fstate_file;

        fstate_file.open (temp_dir + "/network_state/fstate_0.txt");
        fstate_file << "2,3,0,0,1" << std::endl;
        fstate_file << "0,3,1,0,0" << std::endl;
        fstate_file << "1,3,3,1,0" << std::endl;
        fstate_file.close();

        fstate_file.open (temp_dir + "/network_state/fstate_1000000000.txt");
        fstate_file << "0,3,-1,-1,-1" << std::endl;
        fstate_file.close();

        fstate_file.open (temp_dir + "/network_state/fstate_2000000000.txt");
        fstate_file << "0,3,3,1,0" << std::endl;
        fstate_file.close();

        fstate_file.open (temp_dir + "/network_state/fstate_3000000000.txt");
        fstate_file << "2,3,1,0,1" << std::endl;
        fstate_file.close();

        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);

        // Install the scenario
        setup_scenario(100.0, false, 0.0);

        // Load in the arbiter helper
        ArbiterSingleForwardHelper arbiterHelper(basicSimulation, allNodes);

        // Get the arbiter of node 2
        Ptr<Arbiter> arbiter = allNodes.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter();

        // At the start
        ASSERT_EQUAL(
            arbiter->GetObject<ArbiterSingleForward>()->StringReprOfForwardingState(),
            "Single-forward state of node 2\n"
            "  -> 0: (-2, -2, -2)\n"
            "  -> 1: (-2, -2, -2)\n"
            "  -> 2: (-2, -2, -2)\n"
            "  -> 3: (0, 1, 2)\n"
        );

        // Basic optimization
        TcpOptimizer::OptimizeBasic(basicSimulation);

        //////////////////////
        // UDP application

        // Install a UDP burst client on all
        UdpBurstHelper udpBurstHelper(1026, basicSimulation->GetLogsDir());
        ApplicationContainer udpApp = udpBurstHelper.Install(allNodes);
        udpApp.Start(Seconds(0.0));

        // UDP burst info entry
        UdpBurstInfo udpBurstInfo1(
                0,
                src_udp_id_1,
                dst_udp_id_1,
                burst_1_rate, // Rate in Mbit/s
                0,
                100000000000, // Duration in ns // 100000000000
                "abc",
                "def"
        );
        udpApp.Get(src_udp_id_1)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                udpBurstInfo1,
                InetSocketAddress(allNodes.Get(dst_udp_id_1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                true
        );
        udpApp.Get(dst_udp_id_1)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                udpBurstInfo1,
                true
        );

        // Run simulation
        basicSimulation->Run();

        // At the end
        ASSERT_EQUAL(
                arbiter->GetObject<ArbiterSingleForward>()->StringReprOfForwardingState(),
                "Single-forward state of node 2\n"
                "  -> 0: (-2, -2, -2)\n"
                "  -> 1: (-2, -2, -2)\n"
                "  -> 2: (-2, -2, -2)\n"
                "  -> 3: (1, 1, 2)\n"
        );

        // Incoming counting
        int arrival_0s_to_1s = 0;
        int arrival_1s_to_2s = 0;
        int arrival_2s_to_3s = 0;
        int arrival_3s_to_4s = 0;
        std::vector<std::string> lines_precise_incoming_csv = read_file_direct(temp_dir + "/logs_ns3/udp_burst_0_incoming.csv");
        for (std::string line : lines_precise_incoming_csv) {
            std::vector <std::string> line_spl = split_string(line, ",");
            int64_t timestamp = parse_positive_int64(line_spl[2]);
            if (timestamp < 1000000000) {
                arrival_0s_to_1s += 1;
            } else if (timestamp < 2000000000) {
                arrival_1s_to_2s += 1;
            } else if (timestamp < 3000000000) {
                arrival_2s_to_3s += 1;
            } else if (timestamp < 4000000000) {
                arrival_3s_to_4s += 1;
            }
        }

        // Only an outage in interval [1s, 2s)
        double expected_packets_at_full_rate_over_isl = 4.0 * 1000.0 * 1000.0 / 8.0 / 1500.0;
        double expected_packets_at_full_rate_over_gsl_only = 7.0 * 1000.0 * 1000.0 / 8.0 / 1500.0;
        ASSERT_EQUAL_APPROX(arrival_0s_to_1s, expected_packets_at_full_rate_over_isl, 5);
        ASSERT_EQUAL_APPROX(arrival_1s_to_2s, 100, 5); // 100 packets are still in the ISL queue
        ASSERT_EQUAL_APPROX(arrival_2s_to_3s, expected_packets_at_full_rate_over_gsl_only, 5);
        ASSERT_EQUAL_APPROX(arrival_3s_to_4s, expected_packets_at_full_rate_over_gsl_only, 5);

        // Finalize the simulation
        basicSimulation->Finalize();

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class ManualTwoSatTwoGsChangingRateTest : public ManualTwoSatTwoGsTest {
public:
    ManualTwoSatTwoGsChangingRateTest () : ManualTwoSatTwoGsTest ("manual-two-sat-two-gs changing-rate") {};

    void DoRun () {

        // Retrieve from config
        int src_udp_id_1 = 2;
        int dst_udp_id_1 = 3;
        double burst_1_rate = 100.0;

        const std::string temp_dir = ".tmp-manual-two-sat-two-gs-changing-rate-test";

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);
        mkdir_if_not_exists(temp_dir + "/network_state");

        // Configuration file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=4000000000" << std::endl; // 4s duration
        config_file << "simulation_seed=987654321" << std::endl;
        config_file << "dynamic_state_update_interval_ns=1000000000" << std::endl; // Every 1000ms
        config_file << "satellite_network_routes_dir=network_state" << std::endl;
        config_file << "satellite_network_force_static=false" << std::endl;
        config_file << "gsl_data_rate_megabit_per_s=7.0" << std::endl;
        config_file.close();

        // Forwarding state files
        std::ofstream fstate_file;

        fstate_file.open (temp_dir + "/network_state/fstate_0.txt");
        fstate_file << "2,3,0,0,1" << std::endl;
        fstate_file << "0,3,1,0,0" << std::endl;
        fstate_file << "1,3,3,1,0" << std::endl;
        fstate_file.close();

        fstate_file.open (temp_dir + "/network_state/fstate_1000000000.txt");
        fstate_file << "0,3,-1,-1,-1" << std::endl;
        fstate_file.close();

        fstate_file.open (temp_dir + "/network_state/fstate_2000000000.txt");
        fstate_file << "0,3,3,1,0" << std::endl;
        fstate_file.close();

        fstate_file.open (temp_dir + "/network_state/fstate_3000000000.txt");
        fstate_file << "2,3,1,0,1" << std::endl;
        fstate_file.close();

        // Interface bandwidth files
        std::ofstream gsl_if_bw_file;

        gsl_if_bw_file.open (temp_dir + "/network_state/gsl_if_bandwidth_0.txt");
        gsl_if_bw_file << "0,1,1.0" << std::endl;
        gsl_if_bw_file << "1,1,0.4" << std::endl;
        gsl_if_bw_file << "2,0,1.0" << std::endl;
        gsl_if_bw_file << "3,0,1.0" << std::endl;
        gsl_if_bw_file.close();

        gsl_if_bw_file.open (temp_dir + "/network_state/gsl_if_bandwidth_1000000000.txt");
        gsl_if_bw_file.close();

        gsl_if_bw_file.open (temp_dir + "/network_state/gsl_if_bandwidth_2000000000.txt");
        gsl_if_bw_file << "0,1,2.0" << std::endl;
        gsl_if_bw_file << "2,0,2.0" << std::endl;
        gsl_if_bw_file.close();

        gsl_if_bw_file.open (temp_dir + "/network_state/gsl_if_bandwidth_3000000000.txt");
        gsl_if_bw_file << "2,0,3.0" << std::endl;
        gsl_if_bw_file << "1,1,3.0" << std::endl;
        gsl_if_bw_file.close();

        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);

        // Install the scenario
        setup_scenario(100.0, false, 0.0);

        // Load in the arbiter helper
        ArbiterSingleForwardHelper arbiterHelper(basicSimulation, allNodes);

        // Load in GSL interface bandwidth helper
        GslIfBandwidthHelper gslIfBandwidthHelper(basicSimulation, allNodes);

        // Get the arbiter of node 2
        Ptr<Arbiter> arbiter = allNodes.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter();

        // At the start
        ASSERT_EQUAL(
                arbiter->GetObject<ArbiterSingleForward>()->StringReprOfForwardingState(),
                "Single-forward state of node 2\n"
                "  -> 0: (-2, -2, -2)\n"
                "  -> 1: (-2, -2, -2)\n"
                "  -> 2: (-2, -2, -2)\n"
                "  -> 3: (0, 1, 2)\n"
        );

        // Basic optimization
        TcpOptimizer::OptimizeBasic(basicSimulation);

        //////////////////////
        // UDP application

        // Install a UDP burst client on all
        UdpBurstHelper udpBurstHelper(1026, basicSimulation->GetLogsDir());
        ApplicationContainer udpApp = udpBurstHelper.Install(allNodes);
        udpApp.Start(Seconds(0.0));

        // UDP burst info entry
        UdpBurstInfo udpBurstInfo1(
                0,
                src_udp_id_1,
                dst_udp_id_1,
                burst_1_rate, // Rate in Mbit/s
                0,
                100000000000, // Duration in ns // 100000000000
                "abc",
                "def"
        );
        udpApp.Get(src_udp_id_1)->GetObject<UdpBurstApplication>()->RegisterOutgoingBurst(
                udpBurstInfo1,
                InetSocketAddress(allNodes.Get(dst_udp_id_1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                true
        );
        udpApp.Get(dst_udp_id_1)->GetObject<UdpBurstApplication>()->RegisterIncomingBurst(
                udpBurstInfo1,
                true
        );

        // Run simulation
        basicSimulation->Run();

        // At the end
        ASSERT_EQUAL(
                arbiter->GetObject<ArbiterSingleForward>()->StringReprOfForwardingState(),
                "Single-forward state of node 2\n"
                "  -> 0: (-2, -2, -2)\n"
                "  -> 1: (-2, -2, -2)\n"
                "  -> 2: (-2, -2, -2)\n"
                "  -> 3: (1, 1, 2)\n"
        );

        // Incoming counting
        int arrival_0s_to_1s = 0;
        int arrival_1s_to_2s = 0;
        int arrival_2s_to_3s = 0;
        int arrival_3s_to_4s = 0;
        std::vector<std::string> lines_precise_incoming_csv = read_file_direct(temp_dir + "/logs_ns3/udp_burst_0_incoming.csv");
        for (std::string line : lines_precise_incoming_csv) {
            std::vector <std::string> line_spl = split_string(line, ",");
            int64_t timestamp = parse_positive_int64(line_spl[2]);
            if (timestamp < 1000000000) {
                arrival_0s_to_1s += 1;
            } else if (timestamp < 2000000000) {
                arrival_1s_to_2s += 1;
            } else if (timestamp < 3000000000) {
                arrival_2s_to_3s += 1;
            } else if (timestamp < 4000000000) {
                arrival_3s_to_4s += 1;
            }
        }

        // Only an outage in interval [1s, 2s)
        ASSERT_EQUAL_APPROX(arrival_0s_to_1s, 2.8 * 1000.0 * 1000.0 / 8.0 / 1500.0, 5);
        ASSERT_EQUAL_APPROX(arrival_1s_to_2s, (2.8 / 4.0) * 100.0 + 100.0, 5); // 100 packets are still in the GSL queue, and then the ISL queue is put into the GSL queue at 2.8 Mbit/s (losing 1.2 Mbit/s)
        ASSERT_EQUAL_APPROX(arrival_2s_to_3s, 14.0 * 1000.0 * 1000.0 / 8.0 / 1500.0, 5);
        ASSERT_EQUAL_APPROX(arrival_3s_to_4s, 21.0 * 1000.0 * 1000.0 / 8.0 / 1500.0, 5);

        // Finalize the simulation
        basicSimulation->Finalize();

    }

};

////////////////////////////////////////////////////////////////////////////////////////
