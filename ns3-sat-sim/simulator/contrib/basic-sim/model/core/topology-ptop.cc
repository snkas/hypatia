/*
 * Copyright (c) 2019 ETH Zurich
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
 * Author: Simon
 * Originally based on, but since heavily adapted/extended, the scratch/main authored by Hussain.
 */


#include "topology-ptop.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TopologyPtopQueueSelector);
TypeId TopologyPtopQueueSelector::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::TopologyPtopQueueSelector")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

NS_OBJECT_ENSURE_REGISTERED (TopologyPtopTcQdiscSelector);
TypeId TopologyPtopTcQdiscSelector::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::TopologyPtopTcQdiscSelector")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

}

#include "topology-ptop-queue-selector-default.h"
#include "topology-ptop-tc-qdisc-selector-default.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TopologyPtop);
TypeId TopologyPtop::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::TopologyPtop")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

TopologyPtop::TopologyPtop(
        Ptr<BasicSimulation> basicSimulation,
        const Ipv4RoutingHelper& ipv4RoutingHelper,
        Ptr<TopologyPtopQueueSelector> queueSelector,
        Ptr<TopologyPtopTcQdiscSelector> tcQdiscSelector
) {
    m_basicSimulation = basicSimulation;
    m_queueSelector = queueSelector;
    m_tcQdiscSelector = tcQdiscSelector;
    ReadTopologyConfig();
    ParseTopologyGraph();
    ParseTopologyLinkProperties();
    SetupNodes(ipv4RoutingHelper);
    SetupLinks();
}

TopologyPtop::TopologyPtop(
        Ptr<BasicSimulation> basicSimulation,
        const Ipv4RoutingHelper& ipv4RoutingHelper
) : TopologyPtop(
        basicSimulation,
        ipv4RoutingHelper,
        CreateObject<TopologyPtopQueueSelectorDefault>(),
        CreateObject<TopologyPtopTcQdiscSelectorDefault>()
) {
  // Left empty on purpose
}

/**
 * Read in the topology configuration.
 */
void TopologyPtop::ReadTopologyConfig() {

    // Read the topology configuration
    m_topology_config = read_config(m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("topology_ptop_filename"));
    for (auto const& entry : m_topology_config) {
        if (
                entry.first != "num_nodes" &&
                entry.first != "num_undirected_edges" &&
                entry.first != "switches" &&
                entry.first != "switches_which_are_tors" &&
                entry.first != "servers" &&
                entry.first != "undirected_edges" &&
                entry.first != "all_nodes_are_endpoints" &&
                entry.first != "link_channel_delay_ns" &&
                entry.first != "link_device_data_rate_megabit_per_s" &&
                entry.first != "link_device_queue" &&
                entry.first != "link_interface_traffic_control_qdisc"
        ) {
            throw std::runtime_error("Invalid topology property: " + entry.first);
        }
    }

    // If one want to allow applications on all nodes
    m_all_nodes_are_endpoints = parse_boolean(get_param_or_default("all_nodes_are_endpoints", "false", m_topology_config));

}

/**
 * Parse the topology graph.
 */
void TopologyPtop::ParseTopologyGraph() {
    std::cout << "TOPOLOGY GRAPH PARSING" << std::endl;

    // Basic
    m_num_nodes = parse_positive_int64(get_param_or_fail("num_nodes", m_topology_config));
    m_num_undirected_edges = parse_positive_int64(get_param_or_fail("num_undirected_edges", m_topology_config));

    // Node types
    std::string tmp;
    tmp = get_param_or_fail("switches", m_topology_config);
    m_switches = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_switches, m_num_nodes);
    tmp = get_param_or_fail("switches_which_are_tors", m_topology_config);
    m_switches_which_are_tors = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_switches_which_are_tors, m_num_nodes);
    tmp = get_param_or_fail("servers", m_topology_config);
    m_servers = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_servers, m_num_nodes);

    // Adjacency list
    for (int i = 0; i < m_num_nodes; i++) {
        m_all_node_ids.insert(i);
        m_adjacency_list.push_back(std::set<int64_t>());
    }

    // Edges
    tmp = get_param_or_fail("undirected_edges", m_topology_config);
    std::set<std::string> string_set = parse_set_string(tmp);
    for (std::string s : string_set) {
        std::vector<std::string> spl = split_string(s, "-", 2);
        int64_t a = parse_positive_int64(spl[0]);
        int64_t b = parse_positive_int64(spl[1]);
        if (a == b) {
            throw std::invalid_argument(format_string("Cannot have edge to itself on node %" PRIu64 "", a));
        }
        if (a >= m_num_nodes) {
            throw std::invalid_argument(format_string("Left node identifier in edge does not exist: %" PRIu64 "", a));
        }
        if (b >= m_num_nodes) {
            throw std::invalid_argument(format_string("Right node identifier in edge does not exist: %" PRIu64 "", b));
        }
        if (b < a) {
            throw std::invalid_argument(format_string("As a convention, the left node id must be smaller than the right node id: %" PRIu64 "-%" PRIu64 "", a, b));
        }
        m_undirected_edges.push_back(std::make_pair(a, b));
        m_undirected_edges_set.insert(std::make_pair(a, b));
        m_adjacency_list[a].insert(b);
        m_adjacency_list[b].insert(a);
    }

    // Sort them for convenience
    std::sort(m_undirected_edges.begin(), m_undirected_edges.end());

    // Edge checks

    if (m_undirected_edges.size() != (size_t) m_num_undirected_edges) {
        throw std::invalid_argument("Indicated number of undirected edges does not match edge set");
    }

    if (m_undirected_edges.size() != m_undirected_edges_set.size()) {
        throw std::invalid_argument("Duplicates in edge set");
    }

    // Node type hierarchy checks

    if (!direct_set_intersection(m_servers, m_switches).empty()) {
        throw std::invalid_argument("Server and switch identifiers are not distinct");
    }

    if (direct_set_union(m_servers, m_switches).size() != (size_t) m_num_nodes) {
        throw std::invalid_argument("The servers and switches do not encompass all nodes");
    }

    if (direct_set_intersection(m_switches, m_switches_which_are_tors).size() != m_switches_which_are_tors.size()) {
        throw std::invalid_argument("Servers are marked as ToRs");
    }

    // Servers must be connected to ToRs only
    for (int64_t node_id : m_servers) {
        for (int64_t neighbor_id : m_adjacency_list[node_id]) {
            if (m_switches_which_are_tors.find(neighbor_id) == m_switches_which_are_tors.end()) {
                throw std::invalid_argument(format_string("Server node %" PRId64 " has an edge to node %" PRId64 " which is not a ToR.", node_id, neighbor_id));
            }
        }
    }

    // Check
    if (m_servers.size() > 0) {
        m_has_zero_servers = false;
    } else {
        m_has_zero_servers = true;
    }

    // Check that each node has an assignment to a system id if it is distributed
    if (m_basicSimulation->IsDistributedEnabled()) {
        size_t node_assignment_size = m_basicSimulation->GetDistributedNodeSystemIdAssignment().size();
        if (node_assignment_size != (size_t) m_num_nodes) {
            throw std::invalid_argument(
                    format_string("Incorrect amount of node-to-system-id assignments (must be %" PRId64 " but got %u)", m_num_nodes, node_assignment_size)
            );
        }
    }

    // Print summary
    printf("  > Number of nodes.... %" PRIu64 "\n", m_num_nodes);
    printf("    >> Switches........ %" PRIu64 " (of which %" PRIu64 " are ToRs)\n", m_switches.size(), m_switches_which_are_tors.size());
    printf("    >> Servers......... %" PRIu64 "\n", m_servers.size());
    printf("  > Undirected edges... %" PRIu64 "\n\n", m_num_undirected_edges);
    m_basicSimulation->RegisterTimestamp("Parse topology graph");

}

/**
 * Parse an undirected edge mapping. It checks that exactly for every undirected edge is covered.
 * In an edge definition, the first node identifier must always be lower than the second.
 *
 * @param value     String in the format of "map(a-b: c, ...)"
 *
 * @return Mapping (e.g., { (a, b) : c, ... })
 */
std::map<std::pair<int64_t, int64_t>, std::string> TopologyPtop::ParseUndirectedEdgeMap(std::string value) {
    std::map<std::pair<int64_t, int64_t>, std::string> result;

    // Go over every entry in the mapping
    std::vector<std::pair<std::string, std::string>> pair_list = parse_map_string(value);
    for (std::pair<std::string, std::string> p : pair_list) {

        // Find the a-b
        std::vector<std::string> dash_split_list = split_string(p.first, "-", 2);
        int64_t a = parse_positive_int64(dash_split_list[0]);
        int64_t b = parse_positive_int64(dash_split_list[1]);
        if (b < a) {
            throw std::invalid_argument(format_string("As a convention, the left node id must be smaller than the right node id: %" PRIu64 "-%" PRIu64 "", a, b));
        }

        // No duplicates in the mapping
        if (result.find(std::make_pair(a, b)) != result.end()) {
            throw std::runtime_error("Duplicate key in undirected edge mapping: " + p.first);
        }

        // Must be present
        if (m_undirected_edges_set.find(std::make_pair(a, b)) == m_undirected_edges_set.end()) {
            throw std::runtime_error("Unknown undirected edge: " + p.first);
        }

        // Add to mapping
        result[std::make_pair(a, b)] = p.second;

    }

    if (m_undirected_edges_set.size() != result.size()) {
        throw std::runtime_error("Not all undirected edges were covered");
    }

    return result;
}

/**
 * Parse a directed edge mapping. It checks that exactly for every directed edge (meaning link) is covered.
 *
 * @param value     String in the format of "map(a->b: c, ...)"
 *
 * @return Mapping (e.g., { (a, b) : c, ... })
 */
std::map<std::pair<int64_t, int64_t>, std::string> TopologyPtop::ParseDirectedEdgeMap(std::string value) {
    std::map<std::pair<int64_t, int64_t>, std::string> result;

    // Go over every entry in the mapping
    std::vector<std::pair<std::string, std::string>> pair_list = parse_map_string(value);
    for (std::pair<std::string, std::string> p : pair_list) {

        // Find the a -> b
        std::vector<std::string> dash_split_list = split_string(p.first, "->", 2);
        int64_t a = parse_positive_int64(dash_split_list[0]);
        int64_t b = parse_positive_int64(dash_split_list[1]);

        // No duplicates in the mapping
        if (result.find(std::make_pair(a, b)) != result.end()) {
            throw std::runtime_error("Duplicate key in directed edge mapping: " + p.first);
        }

        // Must be present
        if (m_undirected_edges_set.find(std::make_pair(a < b ? a : b, b > a ? b : a)) == m_undirected_edges_set.end()) {
            throw std::runtime_error("Does not belong to an undirected edge: " + p.first);
        }

        // Add to mapping
        result[std::make_pair(a, b)] = p.second;

    }

    if (m_undirected_edges_set.size() * 2 != result.size()) {
        throw std::runtime_error("Not all directed edges were covered");
    }

    return result;
}

/**
 * Parse the link_channel_delay_ns from the topology configuration.
 *
 * @return Mapping of undirected edge (a, b) to channel delay in nanoseconds
 */
void TopologyPtop::ParseLinkChannelDelayNsProperty() {
    std::string value = get_param_or_fail("link_channel_delay_ns", m_topology_config);

    // Universal value
    if (!starts_with(trim(value), "map")) {

        // Create default mapping
        int64_t link_channel_delay_ns = parse_positive_int64(value);
        for (std::pair<int64_t, int64_t> p : m_undirected_edges_set) {
            m_link_channel_delay_ns_mapping[p] = link_channel_delay_ns;
        }
        std::cout << "    >> Single global value... " << link_channel_delay_ns << " ns" << std::endl;

    } else { // Mapping
        std::map <std::pair<int64_t, int64_t>, std::string> undirected_edge_mapping = ParseUndirectedEdgeMap(value);
        for (auto const& entry : undirected_edge_mapping) {
            m_link_channel_delay_ns_mapping[entry.first] = parse_positive_int64(entry.second);
        }
        std::cout << "    >> Per link channel mapping was read" << std::endl;
    }
}

/**
 * Parse the link_device_data_rate_megabit_per_s from the topology configuration.
 *
 * @return Mapping of directed edge (a, b) (i.e., link) to its device's data rate in Mbit/s
 */
void TopologyPtop::ParseLinkDeviceDataRateMegabitPerSecProperty() {
    std::string value = get_param_or_fail("link_device_data_rate_megabit_per_s", m_topology_config);

    // Default value
    if (!starts_with(trim(value), "map")) {

        // Create default mapping
        double link_device_data_rate_megabit_per_s = parse_positive_double(value);
        for (std::pair<int64_t, int64_t> p : m_undirected_edges_set) {
            m_link_device_data_rate_megabit_per_s_mapping[p] = link_device_data_rate_megabit_per_s;
            m_link_device_data_rate_megabit_per_s_mapping[std::make_pair(p.second, p.first)] = link_device_data_rate_megabit_per_s;
        }
        std::cout << "    >> Single global value... " << link_device_data_rate_megabit_per_s << " Mbit/s" << std::endl;

    } else { // Mapping
        std::map <std::pair<int64_t, int64_t>, std::string> directed_edge_mapping = ParseDirectedEdgeMap(value);
        for (auto const& entry : directed_edge_mapping) {
            m_link_device_data_rate_megabit_per_s_mapping[entry.first] = parse_positive_double(entry.second);
        }
        std::cout << "    >> Per link device mapping was read" << std::endl;
    }
}

/**
 * Parse the link_device_queue from the topology configuration.
 *
 * @return Mapping of directed edge (a, b) (i.e., link) to its device's queue
 */
void TopologyPtop::ParseLinkDeviceQueueProperty() {
    std::string value = get_param_or_fail("link_device_queue", m_topology_config);

    // Default value
    if (!starts_with(trim(value), "map")) {

        // Create default mapping
        std::pair<ObjectFactory, QueueSize> link_device_queue = m_queueSelector->ParseQueueValue(this, value);
        for (std::pair<int64_t, int64_t> p : m_undirected_edges_set) {
            m_link_device_queue_mapping[p] = link_device_queue;
            m_link_device_queue_mapping[std::make_pair(p.second, p.first)] = link_device_queue;
        }
        std::cout << "    >> Single global value... " << link_device_queue.first << std::endl;

    } else { // Mapping
        std::map <std::pair<int64_t, int64_t>, std::string> directed_edge_mapping = ParseDirectedEdgeMap(value);
        for (auto const& entry : directed_edge_mapping) {
            m_link_device_queue_mapping[entry.first] = m_queueSelector->ParseQueueValue(this, entry.second);
        }
        std::cout << "    >> Per link device mapping was read" << std::endl;
    }
}

/**
 * Estimation of the worst case RTT based on the topology and the link settings.
 *
 * MTU = 1500 byte, +2 with the p2p header.
 * There are n_q packets in the queue at most, e.g. n_q + 2 (incl. transmit and within mandatory 1-packet qdisc)
 * Queueing + transmission delay/hop = (n_q + 2) * 1502 byte / link data rate
 * Propagation delay/hop = link delay
 *
 * If the topology is big, lets assume 10 hops either direction worst case, so 20 hops total
 * If the topology is not big, < 10 undirected edges, we just use 2 * number of undirected edges as worst-case hop count
 *
 * num_hops * (((n_q + 2) * 1502 byte) / link data rate) + link delay)
 *
 */
void TopologyPtop::EstimateWorstCaseRtt() {

    // Highest link delay
    int64_t highest_delay_ns = 0;
    for (auto const& entry : m_link_channel_delay_ns_mapping) {
        highest_delay_ns = std::max(highest_delay_ns, entry.second);
    }
    std::cout << "    >> Highest link delay......... " << highest_delay_ns << " ns" << std::endl;

    // Lowest link data rate
    double lowest_data_rate_megabit_per_s = 1000000;
    for (auto const& entry : m_link_device_data_rate_megabit_per_s_mapping) {
        lowest_data_rate_megabit_per_s = std::min(lowest_data_rate_megabit_per_s, entry.second);
    }
    std::cout << "    >> Lowest link data rate...... " << lowest_data_rate_megabit_per_s << " Mbit/s" << std::endl;

    // Highest maximum queue size
    int64_t highest_max_queue_size_byte = 0;
    for (auto const& entry : m_link_device_queue_mapping) {
        QueueSize a = entry.second.second;
        int64_t queue_size_byte;
        if (a.GetUnit() == PACKETS) {
            queue_size_byte = (a.GetValue() + 2) * 1502;
        } else {
            queue_size_byte = a.GetValue() + 2 * 1502;
        }
        highest_max_queue_size_byte = std::max(highest_max_queue_size_byte, queue_size_byte);
    }
    std::cout << "    >> Highest max. queue size.... " << highest_max_queue_size_byte << " byte (assuming 1502 byte frames)" << std::endl;

    // Estimated worst-case per-hop delay
    double worst_case_per_hop_delay_ns = (double) highest_max_queue_size_byte / (double) (lowest_data_rate_megabit_per_s * 125000.0 / 1000000000.0) + highest_delay_ns;
    std::cout << "    >> Worst-case per-hop delay... " << worst_case_per_hop_delay_ns / 1e6 << " ms" << std::endl;

    // Maximum number of hops
    int worst_case_num_hops = std::min((int64_t) 20, m_num_undirected_edges * 2);
    std::cout << "    >> Worst-case hop count....... " << worst_case_num_hops << " (set to at most 20 even if topology is larger)" << std::endl;

    // Final worst-case RTT estimate
    m_worst_case_rtt_estimate_ns = worst_case_num_hops * worst_case_per_hop_delay_ns;
    printf("    >> Estimated worst-case RTT... %.3f ms\n", m_worst_case_rtt_estimate_ns / 1e6);

}

/**
 * Parse the link_interface_traffic_control_qdisc from the topology configuration.
 *
 * @return Mapping of directed edge (a, b) (i.e., link) to its interface's traffic control queueing discipline
 */
void TopologyPtop::ParseLinkInterfaceTrafficControlQdiscProperty() {
    std::string value = get_param_or_fail("link_interface_traffic_control_qdisc", m_topology_config);

    // Default value
    if (!starts_with(trim(value), "map")) {

        // Create default mapping
        std::pair<bool, TrafficControlHelper> link_interface_traffic_control_qdisc = m_tcQdiscSelector->ParseTcQdiscValue(this, value);
        for (std::pair<int64_t, int64_t> p : m_undirected_edges_set) {
            m_link_interface_traffic_control_qdisc_mapping[p] = link_interface_traffic_control_qdisc;
            m_link_interface_traffic_control_qdisc_mapping[std::make_pair(p.second, p.first)] = link_interface_traffic_control_qdisc;
        }
        std::cout << "    >> Single global value... " << value << std::endl;

    } else { // Mapping
        std::map <std::pair<int64_t, int64_t>, std::string> directed_edge_mapping = ParseDirectedEdgeMap(value);
        for (auto const& entry : directed_edge_mapping) {
            m_link_interface_traffic_control_qdisc_mapping[entry.first] = m_tcQdiscSelector->ParseTcQdiscValue(this, entry.second);
        }
        std::cout << "    >> Per link interface mapping was read" << std::endl;
    }

}

/**
 * Parse all topology link property mappings.
 */
void TopologyPtop::ParseTopologyLinkProperties() {
    std::cout << "TOPOLOGY LINK PROPERTIES PARSING" << std::endl;

    // Channel delay
    std::cout << "  > Link channel delays" << std::endl;
    ParseLinkChannelDelayNsProperty();
    m_basicSimulation->RegisterTimestamp("Parse link-to-channel-delay mapping");

    // Device data rate
    std::cout << "  > Link device data rates" << std::endl;
    ParseLinkDeviceDataRateMegabitPerSecProperty();
    m_basicSimulation->RegisterTimestamp("Parse link-to-device-data-rate mapping");

    // Device queue
    std::cout << "  > Link device queues" << std::endl;
    ParseLinkDeviceQueueProperty();
    m_basicSimulation->RegisterTimestamp("Parse link-to-device-queue mapping");

    // Worst-case RTT
    std::cout << "  > Worst-case RTT estimation" << std::endl;
    EstimateWorstCaseRtt();
    m_basicSimulation->RegisterTimestamp("Estimate worst-case RTT (ns)");

    // Interface traffic control qdisc
    std::cout << "  > Link interface traffic control queuing disciplines" << std::endl;
    ParseLinkInterfaceTrafficControlQdiscProperty();
    m_basicSimulation->RegisterTimestamp("Parse link-to-interface-tc-qdisc mapping");

    std::cout << std::endl;
}

/**
 * Setup nodes by creating them if they are part of this system, and installing the internet stack including routing.
 *
 * @param ipv4RoutingHelper     IPv4 routing helper (decides which routing is installed on the node)
 */
void TopologyPtop::SetupNodes(const Ipv4RoutingHelper& ipv4RoutingHelper) {
    std::cout << "SETUP NODES" << std::endl;

    // Creating the nodes in their respective system ID
    std::cout << "  > Creating nodes" << std::endl;
    if (m_basicSimulation->IsDistributedEnabled()) {
        for (int64_t system_id_assigned : m_basicSimulation->GetDistributedNodeSystemIdAssignment()) {
            m_nodes.Create(1, system_id_assigned);
        }
    } else {
        m_nodes.Create(m_num_nodes);
    }
    m_basicSimulation->RegisterTimestamp("Create nodes");

    // Install Internet on all nodes
    std::cout << "  > Installing Internet stack on each node" << std::endl;
    InternetStackHelper internet;
    internet.SetRoutingHelper(ipv4RoutingHelper);
    internet.Install(m_nodes);

    std::cout << std::endl;
    m_basicSimulation->RegisterTimestamp("Install Internet stack (incl. routing) on nodes");
}

/**
 * Setup all the links based on the topological layout and the link mappings.
 */
void TopologyPtop::SetupLinks() {
    std::cout << "SETUP LINKS" << std::endl;

    // Each link is a network on its own
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");

    // Create Links
    std::cout << "  > Installing links" << std::endl;
    m_interface_idxs_for_edges.clear();
    for (std::pair<int64_t, int64_t> undirected_edge : m_undirected_edges) {

        // Retrieve all relevant details
        std::pair<int64_t, int64_t> edge_a_to_b = undirected_edge;
        std::pair<int64_t, int64_t> edge_b_to_a = std::make_pair(undirected_edge.second, undirected_edge.first);

        // Point-to-point helper
        PointToPointAbHelper p2p;
        p2p.SetDeviceAttributeA("DataRate", DataRateValue(DataRate(std::to_string(m_link_device_data_rate_megabit_per_s_mapping.at(edge_a_to_b)) + "Mbps")));
        p2p.SetDeviceAttributeB("DataRate", DataRateValue(DataRate(std::to_string(m_link_device_data_rate_megabit_per_s_mapping.at(edge_b_to_a)) + "Mbps")));
        p2p.SetQueueFactoryA(m_link_device_queue_mapping.at(edge_a_to_b).first);
        p2p.SetQueueFactoryB(m_link_device_queue_mapping.at(edge_b_to_a).first);
        p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(m_link_channel_delay_ns_mapping.at(undirected_edge))));
        NetDeviceContainer container = p2p.Install(m_nodes.Get(undirected_edge.first), m_nodes.Get(undirected_edge.second));

        // Retrieve the network devices installed on either end of the link
        Ptr<PointToPointNetDevice> netDeviceA = container.Get(0)->GetObject<PointToPointNetDevice>();
        Ptr<PointToPointNetDevice> netDeviceB = container.Get(1)->GetObject<PointToPointNetDevice>();

        // Traffic control queueing discipline
        std::pair<bool, TrafficControlHelper> a_to_b_traffic_control_qdisc = m_link_interface_traffic_control_qdisc_mapping.at(edge_a_to_b);
        std::pair<bool, TrafficControlHelper> b_to_a_traffic_control_qdisc = m_link_interface_traffic_control_qdisc_mapping.at(edge_b_to_a);
        if (a_to_b_traffic_control_qdisc.first) {
            a_to_b_traffic_control_qdisc.second.Install(netDeviceA);
        }
        if (b_to_a_traffic_control_qdisc.first) {
            b_to_a_traffic_control_qdisc.second.Install(netDeviceB);
        }

        // Assign IP addresses
        address.Assign(container); // This will install the default traffic control layer if not already done above
        address.NewNetwork();

        // Remove the (default) traffic control layer if undesired
        TrafficControlHelper tch_uninstaller;
        if (!a_to_b_traffic_control_qdisc.first) {
            tch_uninstaller.Uninstall(netDeviceA);
        }
        if (!b_to_a_traffic_control_qdisc.first) {
            tch_uninstaller.Uninstall(netDeviceB);
        }

        // Save to mapping
        uint32_t a_if_idx = container.Get(0)->GetIfIndex();
        uint32_t b_if_idx = container.Get(1)->GetIfIndex();
        m_interface_idxs_for_edges.push_back(std::make_pair(a_if_idx, b_if_idx));
        m_net_devices_for_edges.push_back(std::make_pair(netDeviceA, netDeviceB));
        m_link_to_net_device[edge_a_to_b] = netDeviceA;
        m_link_to_net_device[edge_b_to_a] = netDeviceB;

    }

    std::cout << std::endl;
    m_basicSimulation->RegisterTimestamp("Create links and edge-to-interface-index mapping");
}

const NodeContainer& TopologyPtop::GetNodes() {
    return m_nodes;
}

int64_t TopologyPtop::GetNumNodes() {
    return m_num_nodes;
}

int64_t TopologyPtop::GetNumUndirectedEdges() {
    return m_num_undirected_edges;
}

const std::set<int64_t>& TopologyPtop::GetSwitches() {
    return m_switches;
}

const std::set<int64_t>& TopologyPtop::GetSwitchesWhichAreTors() {
    return m_switches_which_are_tors;
}

const std::set<int64_t>& TopologyPtop::GetServers() {
    return m_servers;
}

bool TopologyPtop::IsValidEndpoint(int64_t node_id) {
    if (m_all_nodes_are_endpoints) {
        return true;
    } else {
        if (m_has_zero_servers) {
            return m_switches_which_are_tors.find(node_id) != m_switches_which_are_tors.end();
        } else {
            return m_servers.find(node_id) != m_servers.end();
        }
    }
}

const std::set<int64_t>& TopologyPtop::GetEndpoints() {
    if (m_all_nodes_are_endpoints) {
        return m_all_node_ids;
    } else {
        if (m_has_zero_servers) {
            return m_switches_which_are_tors;
        } else {
            return m_servers;
        }
    }
}

const std::vector<std::pair<int64_t, int64_t>>& TopologyPtop::GetUndirectedEdges() {
    return m_undirected_edges;
}

const std::set<std::pair<int64_t, int64_t>>& TopologyPtop::GetUndirectedEdgesSet() {
    return m_undirected_edges_set;
}

const std::vector<std::set<int64_t>>& TopologyPtop::GetAllAdjacencyLists() {
    return m_adjacency_list;
}

const std::set<int64_t>& TopologyPtop::GetAdjacencyList(int64_t node_id) {
    return m_adjacency_list[node_id];
}

int64_t TopologyPtop::GetWorstCaseRttEstimateNs() {
    return m_worst_case_rtt_estimate_ns;
}

const std::vector<std::pair<uint32_t, uint32_t>>& TopologyPtop::GetInterfaceIdxsForEdges() {
    return m_interface_idxs_for_edges;
}

const std::vector<std::pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>>& TopologyPtop::GetNetDevicesForEdges() {
    return m_net_devices_for_edges;
}

Ptr<PointToPointNetDevice> TopologyPtop::GetNetDeviceForLink(std::pair<int64_t, int64_t> link) {
    return m_link_to_net_device.at(link);
}

}
