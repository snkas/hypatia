#include "arbiter-ecmp-helper.h"

namespace ns3 {

void ArbiterEcmpHelper::InstallArbiters (Ptr<BasicSimulation> basicSimulation, Ptr<TopologyPtop> topology) {
    std::cout << "SETUP ECMP ROUTING" << std::endl;

    NodeContainer nodes = topology->GetNodes();

    // Calculate and instantiate the routing
    std::cout << "  > Calculating ECMP routing" << std::endl;
    std::vector<std::vector<std::vector<uint32_t>>> global_ecmp_state = CalculateGlobalState(topology);
    basicSimulation->RegisterTimestamp("Calculate ECMP routing state");

    std::cout << "  > Setting the routing arbiter on each node" << std::endl;
    for (int i = 0; i < topology->GetNumNodes(); i++) {
        Ptr<ArbiterEcmp> arbiterEcmp = CreateObject<ArbiterEcmp>(nodes.Get(i), nodes, topology, global_ecmp_state[i]);
        nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiterEcmp);
    }
    basicSimulation->RegisterTimestamp("Setup routing arbiter on each node");

    std::cout << std::endl;
}

// This is static
std::vector<std::vector<std::vector<uint32_t>>> ArbiterEcmpHelper::CalculateGlobalState(Ptr<TopologyPtop> topology) {

    // Final result
    std::vector<std::vector<std::vector<uint32_t>>> global_candidate_list;

    ///////////////////////////
    // Floyd-Warshall

    int64_t n = topology->GetNumNodes();

    // Enforce that more than 40000 nodes is not permitted (sqrt(2^31) ~= 46340, so let's call it an even 40000)
    if (n > 40000) {
        throw std::runtime_error("Cannot handle more than 40000 nodes");
    }

    // Initialize with 0 distance to itself, and infinite distance to all others
    int32_t n2 = n * n;
    int32_t* dist = new int32_t[n2];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                dist[n * i + j] = 0;
            } else {
                dist[n * i + j] = 100000000;
            }
        }
    }

    // If there is an edge, the distance is 1
    for (std::pair<int64_t, int64_t> edge : topology->GetUndirectedEdges()) {
        dist[n * edge.first + edge.second] = 1;
        dist[n * edge.second + edge.first] = 1;
    }

    // Floyd-Warshall core
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (dist[n * i + j] > dist[n * i + k] + dist[n * k + j]) {
                    dist[n * i + j] = dist[n * i + k] + dist[n * k + j];
                }
            }
        }
    }

    ///////////////////////////
    // Determine from the shortest path distances
    // the possible next hops

    // ECMP candidate list: candidate_list[current][destination] = [ list of next hops ]
    global_candidate_list.reserve(topology->GetNumNodes());
    for (int i = 0; i < topology->GetNumNodes(); i++) {
        std::vector<std::vector<uint32_t>> v;
        v.reserve(topology->GetNumNodes());
        for (int j = 0; j < topology->GetNumNodes(); j++) {
            v.push_back(std::vector<uint32_t>());
        }
        global_candidate_list.push_back(v);
    }

    // Candidate next hops are determined in the following way:
    // For each edge a -> b, for a destination t:
    // If the shortest_path_distance(b, t) == shortest_path_distance(a, t) - 1
    // then a -> b must be part of a shortest path from a towards t.
    for (std::pair<int64_t, int64_t> edge : topology->GetUndirectedEdges()) {
        for (int j = 0; j < n; j++) {
            if (dist[edge.first * n + j] - 1 == dist[edge.second * n + j]) {
                global_candidate_list[edge.first][j].push_back(edge.second);
            }
            if (dist[edge.second * n + j] - 1 == dist[edge.first * n + j]) {
                global_candidate_list[edge.second][j].push_back(edge.first);
            }
        }
    }

    // Free up the distance matrix
    delete[] dist;

    // Return the final global candidate list
    return global_candidate_list;

}

} // namespace ns3
