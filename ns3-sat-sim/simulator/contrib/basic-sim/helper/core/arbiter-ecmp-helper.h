#ifndef ARBITER_ECMP_HELPER_H
#define ARBITER_ECMP_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/basic-simulation.h"
#include "ns3/topology-ptop.h"
#include "ns3/ipv4-arbiter-routing.h"
#include "ns3/arbiter-ecmp.h"

namespace ns3 {

    class ArbiterEcmpHelper
    {
    public:
        static void InstallArbiters (Ptr<BasicSimulation> basicSimulation, Ptr<TopologyPtop> topology);
    private:
        static std::vector<std::vector<std::vector<uint32_t>>> CalculateGlobalState(Ptr<TopologyPtop> topology);
    };

} // namespace ns3

#endif /* ARBITER_ECMP_HELPER_H */
