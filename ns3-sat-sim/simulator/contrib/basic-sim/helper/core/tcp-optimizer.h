#ifndef TCP_OPTIMIZER_H
#define TCP_OPTIMIZER_H

#include "ns3/basic-simulation.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

    class TcpOptimizer
    {
    public:
        static void OptimizeBasic(Ptr<BasicSimulation> basicSimulation);
        static void OptimizeUsingWorstCaseRtt(Ptr<BasicSimulation> basicSimulation, int64_t worst_case_rtt_ns);
    private:
        static void Generic();
    };

} // namespace ns3

#endif /* TCP_OPTIMIZER_H */
