#ifndef ARBITER_H
#define ARBITER_H


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
#include "ns3/topology.h"
#include "ns3/node-container.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/exp-util.h"

namespace ns3 {

class ArbiterResult {

public:
    ArbiterResult(bool failed, uint32_t out_if_idx, uint32_t gateway_ip_address);
    bool Failed();
    uint32_t GetOutIfIdx();
    uint32_t GetGatewayIpAddress();

private:
    bool m_failed;
    uint32_t m_out_if_idx;
    uint32_t m_gateway_ip_address;

};

class Arbiter : public Object
{

public:
    static TypeId GetTypeId (void);
    Arbiter(Ptr<Node> this_node, NodeContainer nodes);

    /**
     * Resolve the node identifier from an IP address.
     *
     * @param ip    IP address
     *
     * @return Node identifier
     */
    uint32_t ResolveNodeIdFromIp(uint32_t ip);

    /**
     * Base decide how to forward. Directly called by ipv4-arbiter-routing.
     * It does some nice pre-processing and checking and calls Decide() of the
     * subclass to actually make the decision.
     *
     * @param pkt                               Packet
     * @param ipHeader                          IP header of the packet
     *
     * @return Routing arbiter result.
     */
    ArbiterResult BaseDecide(
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader
    );

    /**
     * Decide what should be done with the result.
     *
     * @param source_node_id                    Node where the packet originated from
     * @param target_node_id                    Node where the packet has to go to
     * @param pkt                               Packet
     * @param ipHeader                          IP header of the packet
     * @param is_socket_request_for_source_ip   True iff there is not actually forwarding being done, but it is only
     *                                          a dummy packet sent by the socket to decide the source IP address.
     *                                          Most importantly, this means that THERE IS NO OTHER HEADER IN THE
     *                                          PACKET, IT IS EMPTY (EVEN IF THE PROTOCOL FIELD IS SET IN THE IP
     *                                          HEADER).
     *
     * @return Routing arbiter result.
     *
     *         If it is a socket request for source IP AND you signal it failed, it is not a drop but it will send
     *         a direct signal to the socket that there is no route, likely terminating the socket directly.
     *         In other cases, when you set it failed, it leads to a drop.
     *
     *         A TCP socket first asks for source IP, and then subsequently the tcp-l4 layer does another
     *         call with the full header to get the real decision.
     *
     *         A UDP source only asks for source IP, and does not do another subsequent call in the udp-l4 layer.
     */
    virtual ArbiterResult Decide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) = 0;

    /**
     * Convert the forwarding state (i.e., routing table) to a string representation.
     *
     * @return String representation
     */
    virtual std::string StringReprOfForwardingState() = 0;

protected:
    int32_t m_node_id;
    ns3::NodeContainer m_nodes;

private:
    std::map<uint32_t, uint32_t> m_ip_to_node_id;
    std::map<uint32_t, uint32_t>::iterator m_ip_to_node_id_it;

};

}

#endif //ARBITER_H
