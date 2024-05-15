/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * (Based on PtopLinkUtilizationTracker)
 * Author: Simon
 */

#ifndef PTOP_LINK_QUEUE_TRACKER_HELPER_H
#define PTOP_LINK_QUEUE_TRACKER_HELPER_H

#include "ns3/basic-simulation.h"
#include "ns3/topology-ptop.h"
#include "ns3/ptop-link-queue-tracker.h"

namespace ns3 {

    class PtopLinkQueueTrackerHelper
    {

    public:
        PtopLinkQueueTrackerHelper(Ptr<BasicSimulation> basicSimulation, Ptr<TopologyPtop> topology);
        void WriteResults();

    private:
        std::vector<std::pair<std::pair<int64_t, int64_t>, Ptr<PtopLinkQueueTracker>>> m_queue_trackers;
        Ptr<BasicSimulation> m_basicSimulation;
        Ptr<TopologyPtop> m_topology;
        bool m_enabled;

        std::string m_filename_link_queue_pkt_csv;
        std::string m_filename_link_queue_byte_csv;

        uint32_t m_system_id;
        bool m_enable_distributed;
        std::vector<int64_t> m_distributed_node_system_id_assignment;

    };


} // namespace ns3

#endif /* PTOP_LINK_QUEUE_TRACKER_HELPER_H */
