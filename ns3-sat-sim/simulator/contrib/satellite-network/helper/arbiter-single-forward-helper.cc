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

ArbiterSingleForwardHelper::ArbiterSingleForwardHelper (Ptr<BasicSimulation> basicSimulation, Ptr<TopologySatelliteNetwork> topology) {
    std::cout << "SETUP SINGLE FORWARDING ROUTING" << std::endl;
    m_basicSimulation = basicSimulation;
    m_topology = topology;

    // Read in initial forwarding state
    std::cout << "  > Create initial single forwarding state" << std::endl;
    std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> initial_forwarding_state = InitialEmptyForwardingState();
    basicSimulation->RegisterTimestamp("Create initial single forwarding state");

    // Set the routing arbiters
    std::cout << "  > Setting the routing arbiter on each node" << std::endl;
    NodeContainer nodes = m_topology->GetNodes();
    for (int i = 0; i < m_topology->GetNumNodes(); i++) {
        Ptr<ArbiterSingleForward> arbiter = CreateObject<ArbiterSingleForward>(nodes.Get(i), nodes, m_topology, initial_forwarding_state[i]);
        m_arbiters.push_back(arbiter);
        nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
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
    for (int i = 0; i < m_topology->GetNumNodes(); i++) {
        std::vector <std::tuple<int32_t, int32_t, int32_t>> next_hop_list;
        for (int j = 0; j < m_topology->GetNumNodes(); j++) {
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

            // Retrieve node identifiers
            int64_t current_node_id = parse_positive_int64(comma_split[0]);
            int64_t target_node_id = parse_positive_int64(comma_split[1]);
            int64_t next_hop_node_id = parse_int64(comma_split[2]);
            int64_t my_if_id = parse_int64(comma_split[3]);
            int64_t next_if_id = parse_int64(comma_split[4]);
            // TODO: more node identifier checks
            if (next_hop_node_id < -1) {
                throw std::runtime_error("Invalid next hop id.");
            } else if (my_if_id < -1) {
                throw std::runtime_error("Invalid my interface id.");
            } else if (next_if_id < -1) {
                throw std::runtime_error("Invalid next interface id.");
            }

            // Add to forwarding state
            m_arbiters[current_node_id]->SetSingleForwardState(
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
