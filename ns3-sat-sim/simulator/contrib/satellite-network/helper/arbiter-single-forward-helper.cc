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

#include "arbiter-single-forward-helper.h"

namespace ns3 {

ArbiterSingleForwardHelper::ArbiterSingleForwardHelper (Ptr<BasicSimulation> basicSimulation, NodeContainer nodes) {
    std::cout << "SETUP SINGLE FORWARDING ROUTING" << std::endl;
    m_basicSimulation = basicSimulation;
    m_nodes = nodes;

    // Read in initial forwarding state
    std::cout << "  > Create initial single forwarding state" << std::endl;
    std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> initial_forwarding_state = InitialEmptyForwardingState();
    basicSimulation->RegisterTimestamp("Create initial single forwarding state");

    // Set the routing arbiters
    std::cout << "  > Setting the routing arbiter on each node" << std::endl;
    for (size_t i = 0; i < m_nodes.GetN(); i++) {
        Ptr<ArbiterSingleForward> arbiter = CreateObject<ArbiterSingleForward>(m_nodes.Get(i), m_nodes, initial_forwarding_state[i]);
        m_arbiters.push_back(arbiter);
        m_nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
    }
    basicSimulation->RegisterTimestamp("Setup routing arbiter on each node");

    // Load first forwarding state
    m_dynamicStateUpdateIntervalNs = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("dynamic_state_update_interval_ns"));
    std::cout << "  > Forward state update interval: " << m_dynamicStateUpdateIntervalNs << "ns" << std::endl;
    std::cout << "  > Perform first forwarding state load for t=0" << std::endl;
    UpdateForwardingState(0);
    basicSimulation->RegisterTimestamp("Create initial single forwarding state");

    std::cout << std::endl;
}

std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>
ArbiterSingleForwardHelper::InitialEmptyForwardingState() {
    std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> initial_forwarding_state;
    for (size_t i = 0; i < m_nodes.GetN(); i++) {
        std::vector <std::tuple<int32_t, int32_t, int32_t>> next_hop_list;
        for (size_t j = 0; j < m_nodes.GetN(); j++) {
            next_hop_list.push_back(std::make_tuple(-2, -2, -2)); // -2 indicates an invalid entry
        }
        initial_forwarding_state.push_back(next_hop_list);
    }
    return initial_forwarding_state;
}

void ArbiterSingleForwardHelper::UpdateForwardingState(int64_t t) {

    // Filename
    std::ostringstream res;
    res << m_basicSimulation->GetRunDir() << "/";
    res << m_basicSimulation->GetConfigParamOrFail("satellite_network_routes_dir") << "/fstate_" << t << ".txt";
    std::string filename = res.str();

    // Check that the file exists
    if (!file_exists(filename)) {
        throw std::runtime_error(format_string("File %s does not exist.", filename.c_str()));
    }

    // Open file
    std::string line;
    std::ifstream fstate_file(filename);
    if (fstate_file) {

        // Go over each line
        size_t line_counter = 0;
        while (getline(fstate_file, line)) {

            // Split on ,
            std::vector<std::string> comma_split = split_string(line, ",", 5);

            // Retrieve identifiers
            int64_t current_node_id = parse_positive_int64(comma_split[0]);
            int64_t target_node_id = parse_positive_int64(comma_split[1]);
            int64_t next_hop_node_id = parse_int64(comma_split[2]);
            int64_t my_if_id = parse_int64(comma_split[3]);
            int64_t next_if_id = parse_int64(comma_split[4]);

            // Check the node identifiers
            NS_ABORT_MSG_IF(current_node_id < 0 || current_node_id >= m_nodes.GetN(), "Invalid current node id.");
            NS_ABORT_MSG_IF(target_node_id < 0 || target_node_id >= m_nodes.GetN(), "Invalid target node id.");
            NS_ABORT_MSG_IF(next_hop_node_id < -1 || next_hop_node_id >= m_nodes.GetN(), "Invalid next hop node id.");

            // Drops are only valid if all three values are -1
            NS_ABORT_MSG_IF(
                    !(next_hop_node_id == -1 && my_if_id == -1 && next_if_id == -1)
                    &&
                    !(next_hop_node_id != -1 && my_if_id != -1 && next_if_id != -1),
                    "All three must be -1 for it to signify a drop."
            );

            // Check the interfaces exist
            NS_ABORT_MSG_UNLESS(my_if_id == -1 || (my_if_id >= 0 && my_if_id + 1 < m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNInterfaces()), "Invalid current interface");
            NS_ABORT_MSG_UNLESS(next_if_id == -1 || (next_if_id >= 0 && next_if_id + 1 < m_nodes.Get(next_hop_node_id)->GetObject<Ipv4>()->GetNInterfaces()), "Invalid next hop interface");

            // Node id and interface id checks are only necessary for non-drops
            if (next_hop_node_id != -1 && my_if_id != -1 && next_if_id != -1) {

                // It must be either GSL or ISL
                bool source_is_gsl = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<GSLNetDevice>() != 0;
                bool source_is_isl = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<PointToPointLaserNetDevice>() != 0;
                NS_ABORT_MSG_IF((!source_is_gsl) && (!source_is_isl), "Only GSL and ISL network devices are supported");

                // If current is a GSL interface, the destination must also be a GSL interface
                NS_ABORT_MSG_IF(
                    source_is_gsl &&
                    m_nodes.Get(next_hop_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + next_if_id)->GetObject<GSLNetDevice>() == 0,
                    "Destination interface must be attached to a GSL network device"
                );

                // If current is a p2p laser interface, the destination must match exactly its counter-part
                NS_ABORT_MSG_IF(
                    source_is_isl &&
                    m_nodes.Get(next_hop_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + next_if_id)->GetObject<PointToPointLaserNetDevice>() == 0,
                    "Destination interface must be an ISL network device"
                );
                if (source_is_isl) {
                    Ptr<NetDevice> device0 = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<PointToPointLaserNetDevice>()->GetChannel()->GetDevice(0);
                    Ptr<NetDevice> device1 = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<PointToPointLaserNetDevice>()->GetChannel()->GetDevice(1);
                    Ptr<NetDevice> other_device = device0->GetNode()->GetId() == current_node_id ? device1 : device0;
                    NS_ABORT_MSG_IF(other_device->GetNode()->GetId() != next_hop_node_id, "Next hop node id across does not match");
                    NS_ABORT_MSG_IF(other_device->GetIfIndex() != 1 + next_if_id, "Next hop interface id across does not match");
                }

            }

            // Add to forwarding state
            m_arbiters.at(current_node_id)->SetSingleForwardState(
                    target_node_id,
                    next_hop_node_id,
                    1 + my_if_id,   // Skip the loop-back interface
                    1 + next_if_id  // Skip the loop-back interface
            );

            // Next line
            line_counter++;

        }

        // Close file
        fstate_file.close();

    } else {
        throw std::runtime_error(format_string("File %s could not be read.", filename.c_str()));
    }

    // Given that this code will only be used with satellite networks, this is okay-ish,
    // but it does create a very tight coupling between the two -- technically this class
    // can be used for other purposes as well
    if (!parse_boolean(m_basicSimulation->GetConfigParamOrDefault("satellite_network_force_static", "false"))) {

        // Plan the next update
        int64_t next_update_ns = t + m_dynamicStateUpdateIntervalNs;
        if (next_update_ns < m_basicSimulation->GetSimulationEndTimeNs()) {
            Simulator::Schedule(NanoSeconds(m_dynamicStateUpdateIntervalNs), &ArbiterSingleForwardHelper::UpdateForwardingState, this, next_update_ns);
        }

    }

}

} // namespace ns3
