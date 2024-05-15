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
 */

#ifndef TOPOLOGY_PTOP_TC_QDISC_SELECTOR_DEFAULT_H
#define TOPOLOGY_PTOP_TC_QDISC_SELECTOR_DEFAULT_H

#include "ns3/topology-ptop.h"

namespace ns3 {

    class TopologyPtopTcQdiscSelectorDefault : public TopologyPtopTcQdiscSelector {
    public:
        static TypeId GetTypeId(void);
        std::pair<bool, TrafficControlHelper> ParseTcQdiscValue(Ptr<TopologyPtop> topology, std::string value);
    };

}

#endif //TOPOLOGY_PTOP_TC_QDISC_SELECTOR_DEFAULT_H
