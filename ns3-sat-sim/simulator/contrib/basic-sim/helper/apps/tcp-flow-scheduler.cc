/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "tcp-flow-scheduler.h"

namespace ns3 {

void TcpFlowScheduler::StartNextFlow(int i) {

    // Fetch the flow to start
    TcpFlowScheduleEntry& entry = m_schedule[i];
    int64_t now_ns = Simulator::Now().GetNanoSeconds();
    NS_ASSERT(now_ns == entry.GetStartTimeNs());

    // Helper to install the source application
    TcpFlowSendHelper source(
            "ns3::TcpSocketFactory",
            InetSocketAddress(m_nodes.Get(entry.GetToNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
            entry.GetSizeByte(),
            entry.GetTcpFlowId(),
            m_enable_logging_for_tcp_flow_ids.find(entry.GetTcpFlowId()) != m_enable_logging_for_tcp_flow_ids.end(),
            m_basicSimulation->GetLogsDir(),
            entry.GetAdditionalParameters()
    );

    // Install it on the node and start it right now
    ApplicationContainer app = source.Install(m_nodes.Get(entry.GetFromNodeId()));
    app.Start(NanoSeconds(0));
    m_apps.push_back(app);

    // If there is a next flow to start, schedule its start
    if (i + 1 != (int) m_schedule.size()) {
        int64_t next_flow_ns = m_schedule[i + 1].GetStartTimeNs();
        Simulator::Schedule(NanoSeconds(next_flow_ns - now_ns), &TcpFlowScheduler::StartNextFlow, this, i + 1);
    }

}

TcpFlowScheduler::TcpFlowScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology) {
    printf("TCP FLOW SCHEDULER\n");

    m_basicSimulation = basicSimulation;
    m_topology = topology;

    // Check if it is enabled explicitly
    m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_tcp_flow_scheduler", "false"));
    if (!m_enabled) {
        std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

    } else {
        std::cout << "  > TCP flow scheduler is enabled" << std::endl;

        // Properties we will use often
        m_nodes = m_topology->GetNodes();
        m_simulation_end_time_ns = m_basicSimulation->GetSimulationEndTimeNs();
        m_enable_logging_for_tcp_flow_ids = parse_set_positive_int64(
                m_basicSimulation->GetConfigParamOrDefault("tcp_flow_enable_logging_for_tcp_flow_ids", "set()"));
        m_system_id = m_basicSimulation->GetSystemId();
        m_enable_distributed = m_basicSimulation->IsDistributedEnabled();
        m_distributed_node_system_id_assignment = m_basicSimulation->GetDistributedNodeSystemIdAssignment();

        // Read schedule
        std::vector<TcpFlowScheduleEntry> complete_schedule = read_tcp_flow_schedule(
                m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("tcp_flow_schedule_filename"),
                m_topology,
                m_simulation_end_time_ns
        );

        // Check that the TCP flow IDs exist in the logging
        for (int64_t tcp_flow_id : m_enable_logging_for_tcp_flow_ids) {
            if ((size_t) tcp_flow_id >= complete_schedule.size()) {
                throw std::invalid_argument("Invalid TCP flow ID in tcp_flow_enable_logging_for_tcp_flow_ids: " + std::to_string(tcp_flow_id));
            }
        }

        // Filter the schedule to only have applications starting at nodes which are part of this system
        if (m_enable_distributed) {
            std::vector<TcpFlowScheduleEntry> filtered_schedule;
            for (TcpFlowScheduleEntry &entry : complete_schedule) {
                if (m_distributed_node_system_id_assignment[entry.GetFromNodeId()] == m_system_id) {
                    filtered_schedule.push_back(entry);
                }
            }
            m_schedule = filtered_schedule;
        } else {
            m_schedule = complete_schedule;
        }

        // Schedule read
        printf("  > Read schedule (total flow start events: %lu)\n", m_schedule.size());
        m_basicSimulation->RegisterTimestamp("Read flow schedule");

        // Determine filenames
        if (m_enable_distributed) {
            m_flows_csv_filename =
                    m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_tcp_flows.csv";
            m_flows_txt_filename =
                    m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_tcp_flows.txt";
        } else {
            m_flows_csv_filename = m_basicSimulation->GetLogsDir() + "/tcp_flows.csv";
            m_flows_txt_filename = m_basicSimulation->GetLogsDir() + "/tcp_flows.txt";
        }

        // Remove files if they are there
        remove_file_if_exists(m_flows_csv_filename);
        remove_file_if_exists(m_flows_txt_filename);
        printf("  > Removed previous flow log files if present\n");
        m_basicSimulation->RegisterTimestamp("Remove previous flow log files");

        // Install sink on each endpoint node
        std::cout << "  > Setting up TCP flow sinks" << std::endl;
        for (int64_t endpoint : m_topology->GetEndpoints()) {
            if (!m_enable_distributed || m_distributed_node_system_id_assignment[endpoint] == m_system_id) {
                TcpFlowSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 1024));
                ApplicationContainer app = sink.Install(m_nodes.Get(endpoint));
                app.Start(Seconds(0.0));
            }
        }
        m_basicSimulation->RegisterTimestamp("Setup TCP flow sinks");

        // Setup start of first source application
        std::cout << "  > Setting up traffic TCP flow starter" << std::endl;
        if (m_schedule.size() > 0) {
            Simulator::Schedule(NanoSeconds(m_schedule[0].GetStartTimeNs()), &TcpFlowScheduler::StartNextFlow, this, 0);
        }
        m_basicSimulation->RegisterTimestamp("Setup traffic TCP flow starter");

    }

    std::cout << std::endl;
}

void TcpFlowScheduler::WriteResults() {
    std::cout << "STORE TCP FLOW RESULTS" << std::endl;

    // Check if it is enabled explicitly
    if (!m_enabled) {
        std::cout << "  > Not enabled, so no TCP flow results are written" << std::endl;

    } else {

        // Open files
        std::cout << "  > Opening TCP flow log files:" << std::endl;
        FILE* file_csv = fopen(m_flows_csv_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_flows_csv_filename << std::endl;
        FILE* file_txt = fopen(m_flows_txt_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_flows_txt_filename << std::endl;

        // Header
        std::cout << "  > Writing tcp_flows.txt header" << std::endl;
        fprintf(
                file_txt, "%-16s%-10s%-10s%-16s%-18s%-18s%-16s%-16s%-13s%-16s%-14s%s\n",
                "TCP Flow ID", "Source", "Target", "Size", "Start time (ns)",
                "End time (ns)", "Duration", "Sent", "Progress", "Avg. rate", "Finished?", "Metadata"
        );

        // Go over the schedule, write each flow's result
        std::cout << "  > Writing log files line-by-line" << std::endl;
        std::cout << "  > Total TCP flow log entries to write... " << m_apps.size() << std::endl;
        uint32_t app_idx = 0;
        for (TcpFlowScheduleEntry& entry : m_schedule) {

            // Retrieve application
            Ptr<TcpFlowSendApplication> flowSendApp = m_apps.at(app_idx).Get(0)->GetObject<TcpFlowSendApplication>();

            // Finalize the detailed logs (if they are enabled)
            flowSendApp->FinalizeDetailedLogs();

            // Retrieve statistics
            bool is_completed = flowSendApp->IsCompleted();
            bool is_conn_failed = flowSendApp->IsConnFailed();
            bool is_closed_err = flowSendApp->IsClosedByError();
            bool is_closed_normal = flowSendApp->IsClosedNormally();
            int64_t sent_byte = flowSendApp->GetAckedBytes();
            int64_t fct_ns;
            if (is_completed) {
                fct_ns = flowSendApp->GetCompletionTimeNs() - entry.GetStartTimeNs();
            } else {
                fct_ns = m_simulation_end_time_ns - entry.GetStartTimeNs();
            }
            std::string finished_state;
            if (is_completed) {
                finished_state = "YES";
            } else if (is_conn_failed) {
                finished_state = "NO_CONN_FAIL";
            } else if (is_closed_normal) {
                finished_state = "NO_BAD_CLOSE";
            } else if (is_closed_err) {
                finished_state = "NO_ERR_CLOSE";
            } else {
                finished_state = "NO_ONGOING";
            }

            // Write plain to the csv
            fprintf(
                    file_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%s,%s\n",
                    entry.GetTcpFlowId(), entry.GetFromNodeId(), entry.GetToNodeId(), entry.GetSizeByte(), entry.GetStartTimeNs(),
                    entry.GetStartTimeNs() + fct_ns, fct_ns, sent_byte, finished_state.c_str(), entry.GetMetadata().c_str()
            );

            // Write nicely formatted to the text
            char str_size_megabit[100];
            sprintf(str_size_megabit, "%.2f Mbit", byte_to_megabit(entry.GetSizeByte()));
            char str_duration_ms[100];
            sprintf(str_duration_ms, "%.2f ms", nanosec_to_millisec(fct_ns));
            char str_sent_megabit[100];
            sprintf(str_sent_megabit, "%.2f Mbit", byte_to_megabit(sent_byte));
            char str_progress_perc[100];
            sprintf(str_progress_perc, "%.1f%%", ((double) sent_byte) / ((double) entry.GetSizeByte()) * 100.0);
            char str_avg_rate_megabit_per_s[100];
            sprintf(str_avg_rate_megabit_per_s, "%.1f Mbit/s", byte_to_megabit(sent_byte) / nanosec_to_sec(fct_ns));
            fprintf(
                    file_txt, "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-16s%-18" PRId64 "%-18" PRId64 "%-16s%-16s%-13s%-16s%-14s%s\n",
                    entry.GetTcpFlowId(), entry.GetFromNodeId(), entry.GetToNodeId(), str_size_megabit, entry.GetStartTimeNs(),
                    entry.GetStartTimeNs() + fct_ns, str_duration_ms, str_sent_megabit, str_progress_perc, str_avg_rate_megabit_per_s,
                    finished_state.c_str(), entry.GetMetadata().c_str()
            );

            // Move on application index
            app_idx += 1;

        }

        // Close files
        std::cout << "  > Closing TCP flow log files:" << std::endl;
        fclose(file_csv);
        std::cout << "    >> Closed: " << m_flows_csv_filename << std::endl;
        fclose(file_txt);
        std::cout << "    >> Closed: " << m_flows_txt_filename << std::endl;

        // Register completion
        std::cout << "  > TCP flow log files have been written" << std::endl;
        m_basicSimulation->RegisterTimestamp("Write TCP flow log files");

    }

    std::cout << std::endl;
}

}
