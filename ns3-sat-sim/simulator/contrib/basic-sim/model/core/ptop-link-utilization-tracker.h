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
 * Author: Simon, Hanjing
 */

#ifndef PTOP_LINK_UTILIZATION_TRACKER_H
#define PTOP_LINK_UTILIZATION_TRACKER_H

#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <stdexcept>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"


namespace ns3 {

    class PtopLinkUtilizationTracker : public Object {

    private:

        // Parameters
        int64_t m_interval_ns;

        // State
        int64_t m_prev_time_ns;
        int64_t m_current_interval_start;
        int64_t m_current_interval_end;
        int64_t m_idle_time_counter_ns;
        int64_t m_busy_time_counter_ns;
        bool m_current_state_is_on;
        std::vector<std::tuple<int64_t, int64_t, int64_t>> m_intervals;

    public:
        static TypeId GetTypeId (void);
        PtopLinkUtilizationTracker(Ptr<PointToPointNetDevice> netDevice, int64_t interval_ns);
        void NetDevicePhyTxBeginCallback(Ptr<Packet const>);
        void NetDevicePhyTxEndCallback(Ptr<Packet const>);
        void TrackUtilization(bool next_state_is_on);
        const std::vector<std::tuple<int64_t, int64_t, int64_t>>& FinalizeUtilization();
    };

}

#endif // PTOP_LINK_UTILIZATION_TRACKER_H
