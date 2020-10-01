/*
 * Copyright (c) 2020 ETH Zurich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Simon               2020
 */

#ifndef ARBITER_SATNET_H
#define ARBITER_SATNET_H

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <tuple>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <stdexcept>
#include "ns3/topology-satellite-network.h"
#include "ns3/node-container.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/arbiter.h"

namespace ns3 {

class ArbiterSatnet : public Arbiter
{

public:
    static TypeId GetTypeId (void);
    ArbiterSatnet(Ptr<Node> this_node, NodeContainer nodes);

    // Topology implementation
    ArbiterResult Decide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    /**
     * Decide where the packet needs to be routed to.
     *
     * @param source_node_id                                Node where the packet originated from
     * @param target_node_id                                Node where the packet has to go to
     * @param neighbor_node_ids                             All neighboring nodes from which to choose
     * @param pkt                                           Packet
     * @param ipHeader                                      IP header instance
     * @param is_socket_request_for_source_ip               True iff it is a request for a source IP address,
     *                                                      as such the returning next hop is only used to get the
     *                                                      interface IP address
     *
     * @return Tuple of (next node id, my own interface id, next interface id)
     */
    virtual std::tuple<int32_t, int32_t, int32_t> TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) = 0;

    virtual std::string StringReprOfForwardingState() = 0;

};

}

#endif //ARBITER_SATNET_H
