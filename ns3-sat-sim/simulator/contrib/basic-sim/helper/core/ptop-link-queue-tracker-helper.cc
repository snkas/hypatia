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
 * Author: Simon
 */

#include "ptop-link-queue-tracker-helper.h"

namespace ns3 {

    PtopLinkQueueTrackerHelper::PtopLinkQueueTrackerHelper(Ptr <BasicSimulation> basicSimulation, Ptr <TopologyPtop> topology) {
        std::cout << "POINT-TO-POINT LINK (NET-DEVICE) QUEUE TRACKING" << std::endl;

        // Save for writing results later after simulation is done
        m_basicSimulation = basicSimulation;
        m_topology = topology;

        // Check if it is enabled explicitly
        m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_link_queue_tracking", "false"));
        if (!m_enabled) {
            std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

        } else {
            std::cout << "  > Enabled for all point-to-point links (net-devices) in the topology" << std::endl;

            // Distributed information
            m_system_id = m_basicSimulation->GetSystemId();
            m_enable_distributed = m_basicSimulation->IsDistributedEnabled();
            m_distributed_node_system_id_assignment = m_basicSimulation->GetDistributedNodeSystemIdAssignment();

            // TODO: Add additional parameter to specifically select links

            // Go over every edge in the topology
            for (int i = 0; i < m_topology->GetNumUndirectedEdges(); i++) {

                // Edge a -- b
                std::pair<int64_t, int64_t> edge = m_topology->GetUndirectedEdges()[i];
                std::pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>> edge_net_devices = m_topology->GetNetDevicesForEdges()[i];

                // One tracker a -> b
                if (!m_enable_distributed || m_distributed_node_system_id_assignment[edge.first] == m_system_id) {
                    Ptr<PtopLinkQueueTracker> tracker_a_b = CreateObject<PtopLinkQueueTracker>(edge_net_devices.first);
                    m_queue_trackers.push_back(std::make_pair(edge, tracker_a_b));
                }

                // One tracker b -> a
                if (!m_enable_distributed || m_distributed_node_system_id_assignment[edge.second] == m_system_id) {
                    Ptr<PtopLinkQueueTracker> tracker_b_a = CreateObject<PtopLinkQueueTracker>(edge_net_devices.second);
                    m_queue_trackers.push_back(std::make_pair(std::make_pair(edge.second, edge.first), tracker_b_a));
                }

            }
            std::cout << "  > Tracking link (net-device) queue on " << m_queue_trackers.size() << " point-to-point network devices" << std::endl;
            m_basicSimulation->RegisterTimestamp("Install link (net-device) queue trackers");

            // Determine filenames
            if (m_enable_distributed) {
                m_filename_link_queue_pkt_csv = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_link_queue_pkt.csv";
                m_filename_link_queue_byte_csv = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_link_queue_byte.csv";
            } else {
                m_filename_link_queue_pkt_csv = m_basicSimulation->GetLogsDir() + "/link_queue_pkt.csv";
                m_filename_link_queue_byte_csv = m_basicSimulation->GetLogsDir() + "/link_queue_byte.csv";
            }

            // Remove files if they are there
            remove_file_if_exists(m_filename_link_queue_pkt_csv);

            printf("  > Removed previous link (net-device) queue tracking files if present\n");
            m_basicSimulation->RegisterTimestamp("Remove previous link (net-device) queue tracking log files");

        }

        std::cout << std::endl;
    }

    void PtopLinkQueueTrackerHelper::WriteResults() {
        std::cout << "POINT-TO-POINT LINK (NET-DEVICE) QUEUE TRACKING RESULTS" << std::endl;

        // Check if it is enabled explicitly
        if (!m_enabled) {
            std::cout << "  > Not enabled, so no results are written" << std::endl;

        } else {

            // Open CSV files
            std::cout << "  > Opening link (net-device) queue log files:" << std::endl;
            FILE* file_link_queue_pkt_csv = fopen(m_filename_link_queue_pkt_csv.c_str(), "w+");
            std::cout << "    >> Opened: " << m_filename_link_queue_pkt_csv << std::endl;
            FILE* file_link_queue_byte_csv = fopen(m_filename_link_queue_byte_csv.c_str(), "w+");
            std::cout << "    >> Opened: " << m_filename_link_queue_byte_csv << std::endl;

            // Sort
            struct ascending_by_directed_link
            {
                inline bool operator() (const std::pair<std::pair<int64_t, int64_t>, Ptr<PtopLinkQueueTracker>>& a, const std::pair<std::pair<int64_t, int64_t>, Ptr<PtopLinkQueueTracker>>& b)
                {
                    return (a.first.first == b.first.first ? a.first.second < b.first.second : a.first.first < b.first.first);
                }
            };
            std::sort(m_queue_trackers.begin(), m_queue_trackers.end(), ascending_by_directed_link());

            // Go over every tracker
            std::cout << "  > Writing link (net-device) queue log files" << std::endl;
            for (size_t i = 0; i < m_queue_trackers.size(); i++) {

                // Retrieve the corresponding directed edge
                std::pair<int64_t, int64_t> directed_edge = m_queue_trackers[i].first;

                // Tracker
                Ptr<PtopLinkQueueTracker> tracker = m_queue_trackers[i].second;

                // Queue size in packets
                const std::vector<std::tuple<int64_t, int64_t, int64_t>> log_entries_pkt = tracker->GetIntervalsNumPackets();
                for (size_t j = 0; j < log_entries_pkt.size(); j++) {

                    // Write plain to the CSV file:
                    // <from>,<to>,<interval start (ns)>,<interval end (ns)>,<number of packets>
                    fprintf(file_link_queue_pkt_csv,
                            "%d,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
                            (int) directed_edge.first,
                            (int) directed_edge.second,
                            std::get<0>(log_entries_pkt[j]),
                            std::get<1>(log_entries_pkt[j]),
                            std::get<2>(log_entries_pkt[j])
                    );
                }

                // Queue size in byte
                const std::vector<std::tuple<int64_t, int64_t, int64_t>> log_entries_byte = tracker->GetIntervalsNumBytes();
                for (size_t j = 0; j < log_entries_byte.size(); j++) {

                    // Write plain to the CSV file:
                    // <from>,<to>,<interval start (ns)>,<interval end (ns)>,<number of bytes>
                    fprintf(file_link_queue_byte_csv,
                            "%d,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
                            (int) directed_edge.first,
                            (int) directed_edge.second,
                            std::get<0>(log_entries_byte[j]),
                            std::get<1>(log_entries_byte[j]),
                            std::get<2>(log_entries_byte[j])
                    );
                }


            }

            // Close log files
            std::cout << "  > Closing link (net-device) queue log files:" << std::endl;
            fclose(file_link_queue_pkt_csv);
            std::cout << "    >> Closed: " << m_filename_link_queue_pkt_csv << std::endl;
            fclose(file_link_queue_byte_csv);
            std::cout << "    >> Closed: " << m_filename_link_queue_byte_csv << std::endl;

            // Register completion
            std::cout << "  > Link (net-device) queue log files have been written" << std::endl;
            m_basicSimulation->RegisterTimestamp("Write kink (net-device) queue log files");

        }

        std::cout << std::endl;
    }

}
