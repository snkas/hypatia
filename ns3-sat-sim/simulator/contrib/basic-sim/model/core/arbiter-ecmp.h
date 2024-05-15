#ifndef ARBITER_ECMP_H
#define ARBITER_ECMP_H

#include "ns3/arbiter-ptop.h"
#include "ns3/topology-ptop.h"
#include "ns3/hash.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"

namespace ns3 {

const uint8_t TCP_PROT_NUMBER = 6;
const uint8_t UDP_PROT_NUMBER = 17;

class ArbiterEcmp : public ArbiterPtop
{
public:
    static TypeId GetTypeId (void);

    // Constructor for ECMP forwarding state
    ArbiterEcmp(
            Ptr<Node> this_node,
            NodeContainer nodes,
            Ptr<TopologyPtop> topology,
            std::vector<std::vector<uint32_t>> candidate_list
    );

    // ECMP implementation
    int32_t TopologyPtopDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            const std::set<int64_t>& neighbor_node_ids,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    // ECMP routing table
    std::string StringReprOfForwardingState();

    // Made public for testing
    uint64_t ComputeFiveTupleHash(const Ipv4Header &header, Ptr<const Packet> p, int32_t node_id, bool no_other_headers);

private:
    std::vector<std::vector<uint32_t>> m_candidate_list;
    char m_hash_input_buff[17];
    ns3::Hasher m_hasher;

};

}

#endif //ARBITER_ECMP_H
