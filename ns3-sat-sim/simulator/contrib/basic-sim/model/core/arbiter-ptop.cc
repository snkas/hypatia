#include "ns3/arbiter-ptop.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ArbiterPtop);
TypeId ArbiterPtop::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ArbiterPtop")
            .SetParent<Arbiter> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterPtop::ArbiterPtop(
        Ptr<Node> this_node,
        NodeContainer nodes,
        Ptr<TopologyPtop> topology
) : Arbiter(this_node, nodes) {

    // Topology
    m_topology = topology;

    // Interface indices for all edges in-order
    const std::vector<std::pair<uint32_t, uint32_t>>& interface_idxs_for_edges = topology->GetInterfaceIdxsForEdges();

    // Save which interface is for which neighbor node id
    m_neighbor_node_id_to_if_idx = std::vector<uint32_t>(m_topology->GetNumNodes(), 0);
    for (int i = 0; i < m_topology->GetNumUndirectedEdges(); i++) {
        std::pair<int64_t, int64_t> edge = m_topology->GetUndirectedEdges()[i];
        if (edge.first == m_node_id) {
            m_neighbor_node_id_to_if_idx[edge.second] = interface_idxs_for_edges[i].first;
        } else if (edge.second == m_node_id) {
            m_neighbor_node_id_to_if_idx[edge.first] = interface_idxs_for_edges[i].second;
        }
    }

}

ArbiterResult ArbiterPtop::Decide(
        int32_t source_node_id,
        int32_t target_node_id,
        ns3::Ptr<const ns3::Packet> pkt,
        ns3::Ipv4Header const &ipHeader,
        bool is_socket_request_for_source_ip
) {

    // Decide the next node
    int32_t selected_node_id = TopologyPtopDecide(
                source_node_id,
                target_node_id,
                m_topology->GetAdjacencyList(m_node_id),
                pkt,
                ipHeader,
                is_socket_request_for_source_ip
    );

    // Invalid selected node id
    if (selected_node_id != -1) {

        // Must be a valid node identifier
        if (selected_node_id < 0 || selected_node_id >= m_topology->GetNumNodes()) {
            throw std::runtime_error(format_string(
                    "The selected next node %d is out of node id range of [0, %"  PRId64 ").", selected_node_id, m_topology->GetNumNodes()
            ));
        }

        // Convert the neighbor node id to the interface index of the edge which connects to it
        uint32_t selected_if_idx = m_neighbor_node_id_to_if_idx[selected_node_id];
        if (selected_if_idx == 0) {
            throw std::runtime_error(format_string(
                    "The selected next node %d is not a neighbor of node %d.",
                    selected_node_id,
                    m_node_id
            ));
        }

        // We succeeded in finding the interface to the next hop
        return ArbiterResult(false, selected_if_idx, 0); // Gateway is 0.0.0.0

    } else {
        return ArbiterResult(true, 0, 0); // Failed = no route (means either drop, or socket fails)
    }

}

}
