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

#include "basic-simulation.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BasicSimulation);
TypeId BasicSimulation::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::BasicSimulation")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

BasicSimulation::BasicSimulation(std::string run_dir) {
    m_run_dir = run_dir;
    RegisterTimestamp("Start");

    // Information about the start
    std::cout << "BASIC SIMULATION START" << std::endl;
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer_date[100];
    char buffer_time[100];
    if (std::strftime(buffer_date, sizeof(buffer_date), "%A %B %e, %Y", std::localtime(&now)) && std::strftime(buffer_time, sizeof(buffer_time), "%H:%M:%S", std::localtime(&now))) {
        std::cout << "  > Date... " << buffer_date << std::endl;
        std::cout << "  > Time... " << buffer_time << std::endl;
    }
    std::cout << std::endl;

    ConfigureRunDirectory();
    ReadConfig();
    ConfigureSimulation();
    PrepareBasicSimulationLogFiles();
    WriteFinished(false);
}

int64_t BasicSimulation::NowNsSinceEpoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void BasicSimulation::RegisterTimestamp(std::string label) {
    m_timestamps.push_back(std::make_pair(label, NowNsSinceEpoch()));
}

void BasicSimulation::ConfigureRunDirectory() {
    std::cout << "CONFIGURE RUN DIRECTORY" << std::endl;

    // Run directory
    if (!dir_exists(m_run_dir)) {
        throw std::runtime_error(format_string("Run directory \"%s\" does not exist.", m_run_dir.c_str()));
    } else {
        printf("  > Run directory: %s\n", m_run_dir.c_str());
    }

    // Logs in run directory
    m_logs_dir = m_run_dir + "/logs_ns3";
    mkdir_if_not_exists(m_logs_dir);
    printf("  > Logs directory: %s\n", m_logs_dir.c_str());
    printf("\n");

    RegisterTimestamp("Configure run and logs directory");
}

void BasicSimulation::ReadConfig() {

    // Read the config
    m_config = read_config(m_run_dir + "/config_ns3.properties");

    // Print full config
    printf("CONFIGURATION\n-----\nKEY                                       VALUE\n");
    std::map<std::string, std::string>::iterator it;
    for ( it = m_config.begin(); it != m_config.end(); it++ ) {
        printf("%-40s  %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");

    // End time
    m_simulation_end_time_ns = parse_positive_int64(GetConfigParamOrFail("simulation_end_time_ns"));

    // Seed
    m_simulation_seed = parse_positive_int64(GetConfigParamOrFail("simulation_seed"));

}

void BasicSimulation::ConfigureSimulation() {
    std::cout << "CONFIGURE SIMULATION" << std::endl;

    // Check if enabled
    m_enable_distributed = parse_boolean(GetConfigParamOrDefault("enable_distributed", "false"));
    if (m_enable_distributed) {
        printf("  > Distributed is enabled\n");

        // Bind simulator type
        std::string distributed_simulator_implementation_type = GetConfigParamOrFail("distributed_simulator_implementation_type");
        if (distributed_simulator_implementation_type == "nullmsg") {
            GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::NullMessageSimulatorImpl"));
        } else if (distributed_simulator_implementation_type == "default") {
            GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::DistributedSimulatorImpl"));
        } else {
            throw std::runtime_error(format_string("Unknown distributed simulator implementation type: %s", distributed_simulator_implementation_type));
        }
        printf("  > Simulator implementation type... %s\n", distributed_simulator_implementation_type.c_str());

        // Enable MPI
        int argc = 0;
        char** argv;
        MpiInterface::Enable(&argc, &argv);

        // Retrieve from MPI
        m_system_id = MpiInterface::GetSystemId();
        m_systems_count = MpiInterface::GetSize();

        // Check that the systems count matches up
        uint32_t config_systems_count = (size_t) parse_positive_int64(GetConfigParamOrFail("distributed_systems_count"));
        if (m_systems_count != config_systems_count) {
            throw std::runtime_error(format_string("Systems count in configuration (%u) does not match up with MPI systems count (%u)", config_systems_count, m_systems_count));
        }

        // Check node-to-system-id assignment
        m_distributed_node_system_id_assignment = parse_list_positive_int64(GetConfigParamOrFail("distributed_node_system_id_assignment"));
        std::vector<int> system_id_counter(m_systems_count, 0);
        for (uint32_t i = 0; i < m_distributed_node_system_id_assignment.size(); i++) {
            if (m_distributed_node_system_id_assignment[i] < 0 || m_distributed_node_system_id_assignment[i] >= m_systems_count) {
                throw std::invalid_argument(
                        format_string(
                                "Node %d is assigned to an invalid system id %" PRId64 " (k=%" PRId64 ")",
                        i,
                        m_distributed_node_system_id_assignment[i],
                        m_systems_count
                )
                );
            }
            system_id_counter[m_distributed_node_system_id_assignment[i]]++;
        }

        // All good, showing summary
        printf("  > System information:\n");
        for (uint32_t i = 0; i < m_systems_count; i++) {
            printf("    >> System %d has %d node(s)\n", i, system_id_counter[i]);
        }

    } else {
        printf("  > Distributed is not enabled\n");
        m_distributed_node_system_id_assignment.clear();
        m_system_id = 0;
        m_systems_count = 1;
    }

    // System information
    printf("  > System id........ %u\n", m_system_id);
    printf("  > No. of systems... %u\n", m_systems_count);

    // Set primary seed
    ns3::RngSeedManager::SetSeed(m_simulation_seed);
    std::cout << "  > Seed............. " << m_simulation_seed << std::endl;

    // Set end time
    Simulator::Stop(NanoSeconds(m_simulation_end_time_ns));
    printf("  > Duration......... %.2f s (%" PRId64 " ns)\n", m_simulation_end_time_ns / 1e9, m_simulation_end_time_ns);

    std::cout << std::endl;
    RegisterTimestamp("Configure simulator");
}

void BasicSimulation::PrepareBasicSimulationLogFiles() {
    if (m_enable_distributed) {
        m_finished_filename = m_logs_dir + "/system_" + std::to_string(m_system_id) + "_finished.txt";
        m_timing_results_txt_filename = m_logs_dir + "/system_" + std::to_string(m_system_id) + "_timing_results.txt";
        m_timing_results_csv_filename = m_logs_dir + "/system_" + std::to_string(m_system_id) + "_timing_results.csv";
    } else {
        m_finished_filename = m_logs_dir + "/finished.txt";
        m_timing_results_txt_filename = m_logs_dir + "/timing_results.txt";
        m_timing_results_csv_filename = m_logs_dir + "/timing_results.csv";
    }
    remove_file_if_exists(m_finished_filename);
    remove_file_if_exists(m_timing_results_txt_filename);
    remove_file_if_exists(m_timing_results_csv_filename);
}

void BasicSimulation::WriteFinished(bool finished) {
    std::ofstream fileFinishedEnd(m_finished_filename);
    fileFinishedEnd << (finished ? "Yes" : "No") << std::endl;
    fileFinishedEnd.close();
}

void BasicSimulation::ShowSimulationProgress() {
    int64_t now = NowNsSinceEpoch();
    if (now - m_last_log_time_ns_since_epoch > m_progress_interval_ns) {
        printf(
                "%5.2f%% - Simulation Time = %.2f s ::: Wallclock Time = %.2f s\n",
                (Simulator::Now().GetSeconds() / (m_simulation_end_time_ns / 1e9)) * 100.0,
                Simulator::Now().GetSeconds(),
                (now - m_sim_start_time_ns_since_epoch) / 1e9
        );
        if (m_counter_progress_updates < 8 || m_counter_progress_updates % 5 == 0) { // The first 8 and every 5 progress updates we show estimate
            int remaining_s = (int) ((m_simulation_end_time_ns / 1e9 - Simulator::Now().GetSeconds()) / (Simulator::Now().GetSeconds() / ((now - m_sim_start_time_ns_since_epoch) / 1e9)));
            int seconds = remaining_s % 60;
            int minutes = ((remaining_s - seconds) / 60) % 60;
            int hours = (remaining_s - seconds - minutes * 60) / 3600;
            std::string remainder;
            if (hours > 0) {
                remainder = format_string("Estimated wallclock time remaining: %d hours %d minutes", hours, minutes);
            } else if (minutes > 0) {
                remainder = format_string("Estimated wallclock time remaining: %d minutes %d seconds", minutes, seconds);
            } else {
                remainder = format_string("Estimated wallclock time remaining: %d seconds", seconds);
            }
            std::cout << remainder << std::endl;
        }
        m_last_log_time_ns_since_epoch = now;
        if (m_counter_progress_updates < 4) {
            m_progress_interval_ns = 10000000000; // The first five are every 10s
        } else if (m_counter_progress_updates < 19) {
            m_progress_interval_ns = 20000000000; // The next 15 are every 20s
        } else if (m_counter_progress_updates < 99) {
            m_progress_interval_ns = 60000000000; // The next 80 are every 60s
        } else {
            m_progress_interval_ns = 360000000000; // After that, it is every 360s = 5 minutes
        }
        m_counter_progress_updates++;
    }
    double current_sim_speed_simulation_sec_per_wallclock_sec = Simulator::Now().GetSeconds() / ((now - m_sim_start_time_ns_since_epoch) / 1e9);
    m_simulation_event_interval_s = current_sim_speed_simulation_sec_per_wallclock_sec * (m_progress_interval_ns / 1e9) / 5.0; // At most +20% expected difference to interval
    Simulator::Schedule(Seconds(m_simulation_event_interval_s), &BasicSimulation::ShowSimulationProgress, this);
}

void BasicSimulation::ConfirmAllConfigParamKeysRequested() {
    for (const std::pair<std::string, std::string>& key_val : m_config) {
        if (m_configRequestedKeys.find(key_val.first) == m_configRequestedKeys.end()) {
            throw std::runtime_error(format_string("Config key \'%s\' has not been requested (unused config keys are not allowed)", key_val.first.c_str()));
        }
    }
}

void BasicSimulation::Run() {
    RegisterTimestamp("Whatever is between previous timestamp and the simulation run");
    std::cout << "SIMULATION" << std::endl;

    // Before it starts to run, we need to have processed all the config
    ConfirmAllConfigParamKeysRequested();

    // Schedule progress printing
    m_sim_start_time_ns_since_epoch = NowNsSinceEpoch();
    m_last_log_time_ns_since_epoch = m_sim_start_time_ns_since_epoch;
    Simulator::Schedule(Seconds(m_simulation_event_interval_s), &BasicSimulation::ShowSimulationProgress, this);

    // Run
    printf("Running the simulation for %.2f simulation seconds...\n", (m_simulation_end_time_ns / 1e9));
    Simulator::Run();
    printf("Finished simulation.\n");

    // Print final duration
    printf(
            "Simulation of %.1f seconds took in wallclock time %.1f seconds.\n\n",
            m_simulation_end_time_ns / 1e9,
            (NowNsSinceEpoch() - m_sim_start_time_ns_since_epoch) / 1e9
    );

    RegisterTimestamp("Run simulation");
}

void BasicSimulation::CleanUpSimulation() {
    std::cout << "CLEAN-UP" << std::endl;

    Simulator::Destroy();
    std::cout << "  > Simulator is destroyed" << std::endl;
    RegisterTimestamp("Destroy simulator");

    if (m_enable_distributed) {
        MpiInterface::Disable ();
        std::cout << "  > MPI interface is disabled" << std::endl;
        RegisterTimestamp("Disable MPI interface");
    }

    std::cout << std::endl;
}

void BasicSimulation::StoreTimingResults() {
    std::cout << "TIMING RESULTS" << std::endl;
    std::cout << "------" << std::endl;

    // Write to both files and out
    std::ofstream file_txt(m_timing_results_txt_filename);
    std::ofstream file_csv(m_timing_results_csv_filename);
    int64_t t_prev = -1;
    for (std::pair <std::string, int64_t> &ts : m_timestamps) {
        if (t_prev == -1) {
            t_prev = ts.second;
        } else {

            // Format text line
            std::string line = format_string(
                    "[%7.1f - %7.1f] (%.1f s) :: %s",
                    (t_prev - m_timestamps[0].second) / 1e9,
                    (ts.second - m_timestamps[0].second) / 1e9,
                    (ts.second - t_prev) / 1e9,
                    ts.first.c_str()
            );

            // Standard out (console)
            std::cout << line << std::endl;

            // timing_results.txt
            file_txt << line << std::endl;

            // timing_results.csv (line format: <activity description>,<duration in nanoseconds>)
            file_csv << ts.first << "," << (ts.second - t_prev) << std::endl;

            t_prev = ts.second;
        }
    }
    file_txt.close();
    file_csv.close();

    std::cout << std::endl;
}

void BasicSimulation::Finalize() {
    CleanUpSimulation();
    StoreTimingResults();

    // Information about the end
    std::cout << "BASIC SIMULATION END" << std::endl;
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer_date[100];
    char buffer_time[100];
    if (std::strftime(buffer_date, sizeof(buffer_date), "%A %B %e, %Y", std::localtime(&now)) && std::strftime(buffer_time, sizeof(buffer_time), "%H:%M:%S", std::localtime(&now))) {
        std::cout << "  > Date... " << buffer_date << std::endl;
        std::cout << "  > Time... " << buffer_time << std::endl;
    }
    std::cout << std::endl;

    WriteFinished(true);
}

bool BasicSimulation::IsDistributedEnabled() {
    return m_enable_distributed;
}

uint32_t BasicSimulation::GetSystemId() {
    return m_system_id;
}

uint32_t BasicSimulation::GetSystemsCount() {
    return m_systems_count;
}

std::vector<int64_t> BasicSimulation::GetDistributedNodeSystemIdAssignment() {
    return m_distributed_node_system_id_assignment;
}

int64_t BasicSimulation::GetSimulationEndTimeNs() {
    return m_simulation_end_time_ns;
}

std::string BasicSimulation::GetConfigParamOrFail(std::string key) {
    m_configRequestedKeys.insert(key);
    return get_param_or_fail(key, m_config);
}

std::string BasicSimulation::GetConfigParamOrDefault(std::string key, std::string default_value) {
    m_configRequestedKeys.insert(key);
    return get_param_or_default(key, default_value, m_config);
}

std::string BasicSimulation::GetLogsDir() {
    return m_logs_dir;
}

std::string BasicSimulation::GetRunDir() {
    return m_run_dir;
}

}
