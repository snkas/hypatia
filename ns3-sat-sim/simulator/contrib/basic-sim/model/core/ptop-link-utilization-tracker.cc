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

#include "ptop-link-utilization-tracker.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (PtopLinkUtilizationTracker);
    TypeId PtopLinkUtilizationTracker::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::PtopLinkUtilizationTracker")
                .SetParent<Object> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    PtopLinkUtilizationTracker::PtopLinkUtilizationTracker(Ptr<PointToPointNetDevice> netDevice, int64_t interval_ns) {

        // Register this tracker into the tracing callbacks of the network device
        netDevice->TraceConnectWithoutContext("PhyTxBegin", MakeCallback(&PtopLinkUtilizationTracker::NetDevicePhyTxBeginCallback, this));
        netDevice->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&PtopLinkUtilizationTracker::NetDevicePhyTxEndCallback, this));

        // Interval
        m_interval_ns = interval_ns;

        // Starting state
        m_prev_time_ns = 0;
        m_current_interval_start = 0;
        m_current_interval_end = m_interval_ns;
        m_idle_time_counter_ns = 0;
        m_busy_time_counter_ns = 0;
        m_current_state_is_on = false;

    }

    void PtopLinkUtilizationTracker::NetDevicePhyTxBeginCallback(Ptr<Packet const>) {
        TrackUtilization(true);
    }

    void PtopLinkUtilizationTracker::NetDevicePhyTxEndCallback(Ptr<Packet const>) {
        TrackUtilization(false);
    }

    void PtopLinkUtilizationTracker::TrackUtilization(bool next_state_is_on) {

        // Current time in nanoseconds
        int64_t now_ns = Simulator::Now().GetNanoSeconds();
        while (now_ns >= m_current_interval_end) {

            // Add everything until the end of the interval
            if (next_state_is_on) {
                m_idle_time_counter_ns += m_current_interval_end - m_prev_time_ns;
            } else {
                m_busy_time_counter_ns += m_current_interval_end - m_prev_time_ns;
            }

            // Save into the intervals array
            m_intervals.push_back(std::make_tuple(m_current_interval_start, m_current_interval_end, m_busy_time_counter_ns));

            // This must match up
            NS_ASSERT(m_idle_time_counter_ns + m_busy_time_counter_ns == m_interval_ns);

            // Move to next interval
            m_idle_time_counter_ns = 0;
            m_busy_time_counter_ns = 0;
            m_prev_time_ns = m_current_interval_end;
            m_current_interval_start += m_interval_ns;
            m_current_interval_end += m_interval_ns;

        }

        // If not at the end of a new interval, just keep track of it all
        if (next_state_is_on) {
            m_idle_time_counter_ns += now_ns - m_prev_time_ns;
        } else {
            m_busy_time_counter_ns += now_ns - m_prev_time_ns;
        }

        // This has become the previous call
        m_current_state_is_on = next_state_is_on;
        m_prev_time_ns = now_ns;

    }

    const std::vector<std::tuple<int64_t, int64_t, int64_t>>& PtopLinkUtilizationTracker::FinalizeUtilization() {
        TrackUtilization(!m_current_state_is_on); // End the remaining completed interval(s)

        // The final incomplete interval we also include
        int64_t now_ns = Simulator::Now().GetNanoSeconds();
        if (now_ns != m_current_interval_start) {
            m_intervals.push_back(std::make_tuple(m_current_interval_start, now_ns, m_busy_time_counter_ns));
        }

        return m_intervals;
    }

}
