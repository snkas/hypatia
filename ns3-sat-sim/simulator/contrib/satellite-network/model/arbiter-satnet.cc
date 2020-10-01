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

#include "ns3/arbiter-satnet.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ArbiterSatnet);
TypeId ArbiterSatnet::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ArbiterSatnet")
            .SetParent<Arbiter> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterSatnet::ArbiterSatnet(
        Ptr<Node> this_node,
        NodeContainer nodes
) : Arbiter(this_node, nodes) {
    // Intentionally left empty
}

ArbiterResult ArbiterSatnet::Decide(
        int32_t source_node_id,
        int32_t target_node_id,
        ns3::Ptr<const ns3::Packet> pkt,
        ns3::Ipv4Header const &ipHeader,
        bool is_socket_request_for_source_ip
) {

    // Decide the next node
    std::tuple<int32_t, int32_t, int32_t> next_node_id_my_if_next_if = TopologySatelliteNetworkDecide(
                source_node_id,
                target_node_id,
                pkt,
                ipHeader,
                is_socket_request_for_source_ip
    );

    // Retrieve the components
    int32_t next_node_id = std::get<0>(next_node_id_my_if_next_if);
    int32_t own_if_id = std::get<1>(next_node_id_my_if_next_if);
    int32_t next_if_id = std::get<2>(next_node_id_my_if_next_if);

    // If the result is invalid
    NS_ABORT_MSG_IF(next_node_id == -2 || own_if_id == -2 || next_if_id == -2, "Forwarding state is not set for this node to this target node (invalid).");

    // Check whether it is a drop or not
    if (next_node_id != -1) {

        // Retrieve the IP gateway
        uint32_t select_ip_gateway = m_nodes.Get(next_node_id)->GetObject<Ipv4>()->GetAddress(next_if_id, 0).GetLocal().Get();

        // We succeeded in finding the interface and gateway to the next hop
        return ArbiterResult(false, own_if_id, select_ip_gateway);

    } else {
        return ArbiterResult(true, 0, 0); // Failed = no route (means either drop, or socket fails)
    }

}

}
