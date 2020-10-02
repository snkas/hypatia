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

#ifndef GSL_IF_BANDWIDTH_HELPER
#define GSL_IF_BANDWIDTH_HELPER

#include "ns3/ipv4-routing-helper.h"
#include "ns3/basic-simulation.h"
#include "ns3/topology-satellite-network.h"
#include "ns3/ipv4-arbiter-routing.h"
#include "ns3/arbiter-single-forward.h"

namespace ns3 {

    class GslIfBandwidthHelper
    {
    public:
        GslIfBandwidthHelper(Ptr<BasicSimulation> basicSimulation, NodeContainer nodes);
    private:
        void UpdateGslIfBandwidth(int64_t t);

        // Parameters
        Ptr<BasicSimulation> m_basicSimulation;
        NodeContainer m_nodes;
        double m_gsl_data_rate_megabit_per_s;
        int64_t m_dynamicStateUpdateIntervalNs;

    };

} // namespace ns3

#endif /* GSL_IF_BANDWIDTH_HELPER */
