/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/arbiter-ecmp.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/test.h"
#include "../test-helpers.h"

using namespace ns3;

const std::string arbiter_test_dir = ".tmp-arbiter-test";

void prepare_arbiter_test_config() {
    mkdir_if_not_exists(arbiter_test_dir);
    std::ofstream config_file(arbiter_test_dir + "/config_ns3.properties");
    config_file << "simulation_end_time_ns=10000000000" << std::endl;
    config_file << "simulation_seed=123456789" << std::endl;
    config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
    config_file.close();
}

void prepare_arbiter_test_default_topology() {
    std::ofstream topology_file;
    topology_file.open (arbiter_test_dir + "/topology.properties.temp");
    topology_file << "num_nodes=4" << std::endl;
    topology_file << "num_undirected_edges=4" << std::endl;
    topology_file << "switches=set(0,1,2,3)" << std::endl;
    topology_file << "switches_which_are_tors=set(0,1,2,3)" << std::endl;
    topology_file << "servers=set()" << std::endl;
    topology_file << "undirected_edges=set(0-1,1-2,2-3,0-3)" << std::endl;
    topology_file << "link_channel_delay_ns=10000" << std::endl;
    topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
    topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
    topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
    topology_file.close();
}

void prepare_arbiter_test() {
    prepare_arbiter_test_config();
    prepare_arbiter_test_default_topology();
}

void cleanup_arbiter_test() {
    remove_file_if_exists(arbiter_test_dir + "/config_ns3.properties");
    remove_file_if_exists(arbiter_test_dir + "/topology.properties.temp");
    remove_file_if_exists(arbiter_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(arbiter_test_dir + "/logs_ns3/timing_results.txt");
    remove_file_if_exists(arbiter_test_dir + "/logs_ns3/timing_results.csv");
    remove_dir_if_exists(arbiter_test_dir + "/logs_ns3");
    remove_dir_if_exists(arbiter_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterIpResolutionTestCase : public TestCase
{
public:
    ArbiterIpResolutionTestCase () : TestCase ("routing-arbiter basic") {};
    void DoRun () {
        prepare_arbiter_test();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(arbiter_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForEdges();
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Test valid IPs
        Ptr<Arbiter> arbiter = nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter();
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.0.1").Get()), 0);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.0.2").Get()), 1);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.1.1").Get()), 0);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.1.2").Get()), 3);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.2.1").Get()), 1);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.2.2").Get()), 2);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.3.1").Get()), 2);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.3.2").Get()), 3);

        // All other should be invalid, a few examples
        ASSERT_EXCEPTION(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.0.0").Get()));
        ASSERT_EXCEPTION(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.1.3").Get()));
        ASSERT_EXCEPTION(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.4.1").Get()));

        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

struct ecmp_fields_t {
    int32_t node_id;
    uint32_t src_ip;
    uint32_t dst_ip;
    bool is_tcp;
    bool is_udp;
    uint16_t src_port;
    uint16_t dst_port;
};

void create_headered_packet(Ptr<Packet> p, ecmp_fields_t e) {

    // Set IP header
    Ipv4Header ipHeader;
    ipHeader.SetSource(Ipv4Address(e.src_ip));
    ipHeader.SetDestination(Ipv4Address(e.dst_ip));

    if (e.is_tcp) { // Set TCP+IP header
        TcpHeader tcpHeader;
        tcpHeader.SetSourcePort(e.src_port);
        tcpHeader.SetDestinationPort(e.dst_port);
        ipHeader.SetProtocol(6);
        p->AddHeader(tcpHeader);
        p->AddHeader(ipHeader);

    } else if (e.is_udp) { // Setup UDP+IP header
        UdpHeader udpHeader;
        udpHeader.SetSourcePort(e.src_port);
        udpHeader.SetDestinationPort(e.dst_port);
        ipHeader.SetProtocol(17);
        p->AddHeader(udpHeader);
        p->AddHeader(ipHeader);

    } else { // Setup only IP header
        p->AddHeader(ipHeader);
    }

}

class ArbiterEcmpHashTestCase : public TestCase
{
public:
    ArbiterEcmpHashTestCase () : TestCase ("routing-arbiter-ecmp hash") {};
    void DoRun () {
        prepare_arbiter_test();

        // Create topology
        prepare_arbiter_test_config();
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(arbiter_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForEdges();
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // One example ECMP arbiter
        Ptr<ArbiterEcmp> routingArbiterEcmp = nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter()->GetObject<ArbiterEcmp>();

        ///////
        // All hashes should be different if any of the 5 values are different

        ecmp_fields_t cases[] = {
                {1, 2, 3, true, false, 4, 5}, // TCP
                {6, 2, 3, true, false, 4, 5}, // TCP different node id
                {1, 6, 3, true, false, 4, 5}, // TCP different source IP
                {1, 2, 6, true, false, 4, 5}, // TCP different destination IP
                {1, 2, 3, true, false, 6, 5}, // TCP different source port
                {1, 2, 3, true, false, 4, 6}, // TCP different destination port
                {1, 2, 3, false, true, 4, 5}, // UDP
                {6, 2, 3, false, true, 4, 5}, // UDP different node id
                {1, 6, 3, false, true, 4, 5}, // UDP different source IP
                {1, 2, 6, false, true, 4, 5}, // UDP different destination IP
                {1, 2, 3, false, true, 6, 5}, // UDP different source port
                {1, 2, 3, false, true, 4, 6}, // UDP different destination port
                {1, 2, 3, false, false, 1, 1}, // Not TCP/UDP
                {6, 2, 3, false, false, 1, 1}, // Not TCP/UDP different node id
                {1, 6, 3, false, false, 1, 1}, // Not TCP/UDP different source IP
                {1, 2, 6, false, false, 1, 1}, // Not TCP/UDP different destination IP
        };
        int num_cases = sizeof(cases) / sizeof(ecmp_fields_t);
        std::vector<uint32_t> hash_results;
        std::set<uint32_t> hash_results_set;
        for (ecmp_fields_t e : cases) {
            Ptr<Packet> p = Create<Packet>(5);
            create_headered_packet(p, e);
            Ipv4Header ipHeader;
            p->RemoveHeader(ipHeader);
            hash_results.push_back(routingArbiterEcmp->ComputeFiveTupleHash(ipHeader, p, e.node_id, false));
        }
        for (int i = 0; i < num_cases; i++) {
            for (int j = i + 1; j < num_cases; j++) {
                ASSERT_NOT_EQUAL(hash_results[i], hash_results[j]);
            }
            // std::cout << "Hash[" << i << "] = " << hash_results[i] << std::endl;
        }

        ///////
        // Test same hash

        // Same TCP for same 5-tuple
        Ptr<Packet> p1 = Create<Packet>(555);
        Ptr<Packet> p2 = Create<Packet>(20);
        create_headered_packet(p1, {6666, 4363227, 215326, true, false, 4663, 8888});
        Ipv4Header p1header;
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {6666, 4363227, 215326, true, false, 4663, 8888});
        Ipv4Header p2header;
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 6666, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 6666, false)
        );

        // Same UDP for same 5-tuple
        p1 = Create<Packet>(555);
        p2 = Create<Packet>(8888);
        create_headered_packet(p1, {123456, 32543526, 937383, false, true, 1234, 6737});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {123456, 32543526, 937383, false, true, 1234, 6737});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 123456, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 123456, false)
        );

        // Same not TCP/UDP for same 3-tuple
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, false, false, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, false, false, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, false)
        );

        // TCP: Different source and destination port, so hash should also be different
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, true, false, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, true, false, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_NOT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, false)
        );

        // TCP: If the protocol field is to be ignored because the other headers are said to be not there explicitly
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, true, false, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, true, false, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, true),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, true)
        );

        // UDP: Different source and destination port, so hash should also be different
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, false, true, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, false, true, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_NOT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, false)
        );

        // UDP: If the protocol field is to be ignored because the other headers are said to be not there explicitly
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, false, true, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, false, true, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, true),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, true)
        );

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterEcmpStringReprTestCase : public TestCase
{
public:
    ArbiterEcmpStringReprTestCase () : TestCase ("routing-arbiter-ecmp string-repr") {};
    void DoRun () {
        prepare_arbiter_test();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(arbiter_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForEdges();
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        for (int i = 0; i < topology->GetNumNodes(); i++) {
            std::ostringstream res;
            OutputStreamWrapper out_stream = OutputStreamWrapper(&res);
            nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->PrintRoutingTable(&out_stream);
            std::ostringstream expected;
            if (i == 0) {
                expected << "ECMP state of node 0" << std::endl;
                expected << "  -> 0: {}" << std::endl;
                expected << "  -> 1: {1}" << std::endl;
                expected << "  -> 2: {1,3}" << std::endl;
                expected << "  -> 3: {3}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else if (i == 1) {
                expected << "ECMP state of node 1" << std::endl;
                expected << "  -> 0: {0}" << std::endl;
                expected << "  -> 1: {}" << std::endl;
                expected << "  -> 2: {2}" << std::endl;
                expected << "  -> 3: {0,2}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else if (i == 2) {
                expected << "ECMP state of node 2" << std::endl;
                expected << "  -> 0: {1,3}" << std::endl;
                expected << "  -> 1: {1}" << std::endl;
                expected << "  -> 2: {}" << std::endl;
                expected << "  -> 3: {3}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else if (i == 3) {
                expected << "ECMP state of node 3" << std::endl;
                expected << "  -> 0: {0}" << std::endl;
                expected << "  -> 1: {0,2}" << std::endl;
                expected << "  -> 2: {2}" << std::endl;
                expected << "  -> 3: {}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else {
                ASSERT_TRUE(false);
            }
        }

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

//////////////////////////////////////////////////////////////////////////////////////////

class ArbiterBad: public ArbiterPtop
{
public:

    ArbiterBad(Ptr<Node> this_node, NodeContainer nodes, Ptr<TopologyPtop> topology) : ArbiterPtop(this_node, nodes, topology) {
        // Left empty intentionally
    }

    int32_t TopologyPtopDecide(
        int32_t source_node_id,
        int32_t target_node_id,
        const std::set<int64_t>& neighbor_node_ids,
        ns3::Ptr<const ns3::Packet> pkt,
        ns3::Ipv4Header const &ipHeader,
        bool is_socket_request_for_source_ip
    ) {
        return m_next_decision;
    }

    void SetDecision(int32_t val) {
        m_next_decision = val;
    }

    std::string StringReprOfForwardingState() {
        return "";
    }

private:
    int32_t m_next_decision = -1;

};

class ArbiterBadImplTestCase : public TestCase
{
public:
    ArbiterBadImplTestCase () : TestCase ("routing-arbiter bad-impl") {};
    void DoRun () {
        prepare_arbiter_test();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(arbiter_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForEdges();
        ArbiterBad arbiterBad = ArbiterBad(nodes.Get(1), nodes, topology);

        // This should be fine
        arbiterBad.SetDecision(2);
        Ptr<Packet> p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        Ipv4Header p1header;
        p1->RemoveHeader(p1header);
        ASSERT_EQUAL(2, arbiterBad.BaseDecide(p1, p1header).GetOutIfIdx()); // Should also be interface 2

        // This should also be fine = drop
        arbiterBad.SetDecision(-1);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_TRUE(arbiterBad.BaseDecide(p1, p1header).Failed());

        // Not a neighbor
        arbiterBad.SetDecision(3);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION(arbiterBad.BaseDecide(p1, p1header));

        // Out of range <
        arbiterBad.SetDecision(-2);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION(arbiterBad.BaseDecide(p1, p1header));

        // Out of range >
        arbiterBad.SetDecision(4);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION(arbiterBad.BaseDecide(p1, p1header));

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};
