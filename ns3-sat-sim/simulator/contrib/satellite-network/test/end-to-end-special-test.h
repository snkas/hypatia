/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <map>
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

#include "ns3/basic-simulation.h"
#include "ns3/udp-burst-scheduler.h"
#include "ns3/topology-satellite-network.h"
#include "ns3/tcp-optimizer.h"
#include "ns3/arbiter-single-forward-helper.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/gsl-if-bandwidth-helper.h"

#include "ns3/test.h"
#include "test-helpers.h"

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class EndToEndSpecialTestCase : public TestCase {
public:
    EndToEndSpecialTestCase () : TestCase ("end-to-end-special") {};

    void DoRun () {

        const std::string temp_dir = ".tmp-end-to-end-special-test";
        const std::string dyn_state_dir = temp_dir + "/dynamic_state";

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);
        mkdir_if_not_exists(dyn_state_dir);

        // A configuration file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        int64_t simulation_end_time_ns = 10000000000; // 10s
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=987654321" << std::endl;
        config_file << "satellite_network_dir=." << std::endl;
        config_file << "satellite_network_routes_dir=dynamic_state" << std::endl;
        config_file << "isl_data_rate_megabit_per_s=4.00" << std::endl;
        config_file << "gsl_data_rate_megabit_per_s=10.00" << std::endl;
        config_file << "isl_max_queue_size_pkts=80" << std::endl;
        config_file << "gsl_max_queue_size_pkts=75" << std::endl;
        config_file << "enable_isl_utilization_tracking=true" << std::endl;
        config_file << "isl_utilization_tracking_interval_ns=100000000" << std::endl;
        config_file << "dynamic_state_update_interval_ns=100000000" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=udp_burst_schedule.csv" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=set(0,1)" << std::endl;
        config_file.close();

        // Topology
        //
        // Satellites:               0 ----- 1        2
        //                          ||       ||       |
        //                   ( ......... GSL channel ......... )
        //                    ||    |               |    |
        // Ground stations:   3     4               5    6

        // UDP burst schedule
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (temp_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,3,5,10,0,1000000000000,," << std::endl;
        udp_burst_schedule_file << "1,3,6,10,0,1000000000000,," << std::endl;
        udp_burst_schedule_file << "2,4,5,6,0,1000000000000,," << std::endl;
        udp_burst_schedule_file << "3,4,6,4,0,1000000000000,," << std::endl;
        udp_burst_schedule_file.close();

        // TLES
        std::ofstream tles_file;
        tles_file.open (temp_dir + "/tles.txt");
        tles_file << "1 3" << std::endl;
        tles_file << "Starlink-550 0" << std::endl; // 1477
        tles_file << "1 01478U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    03" << std::endl;
        tles_file << "2 01478  53.0000 335.0000 0000001   0.0000  57.2727 15.19000000    08" << std::endl;
        tles_file << "Starlink-550 1" << std::endl; // 1499
        tles_file << "1 01500U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    09" << std::endl;
        tles_file << "2 01500  53.0000 340.0000 0000001   0.0000  49.0909 15.19000000    01" << std::endl;
        tles_file << "Starlink-550 2" << std::endl; // 1543
        tles_file << "1 01544U 00000ABC 00001.00000000  .00000000  00000-0  00000+0 0    07" << std::endl;
        tles_file << "2 01544  53.0000 350.0000 0000001   0.0000  49.0909 15.19000000    00" << std::endl;
        tles_file.close();

        // ISLs
        std::ofstream isls_file;
        isls_file.open (temp_dir + "/isls.txt");
        isls_file << "0 1" << std::endl;
        isls_file.close();

        // Ground stations
        std::ofstream ground_stations_file;
        ground_stations_file.open (temp_dir + "/ground_stations.txt");
        ground_stations_file << "0,New-York-Newark,40.717042,-74.003663,0.000000,1334103.172127,-4653693.528901,4138656.197504" << std::endl;
        ground_stations_file << "1,New-York-Newark,40.717042,-74.003663,0.000000,1334103.172127,-4653693.528901,4138656.197504" << std::endl;
        ground_stations_file << "2,Atlanta,33.760000,-84.400000,0.000000,517979.453140,-5282763.124122,3524344.845288" << std::endl;
        ground_stations_file << "3,Atlanta,33.760000,-84.400000,0.000000,517979.453140,-5282763.124122,3524344.845288" << std::endl;
        ground_stations_file.close();

        // GSL interfaces info
        std::ofstream gsl_interfaces_info_file;
        gsl_interfaces_info_file.open (temp_dir + "/gsl_interfaces_info.txt");

        // Satellites GSL interfaces info
        gsl_interfaces_info_file << "0,2,2.0" << std::endl;
        gsl_interfaces_info_file << "1,2,2.0" << std::endl;
        gsl_interfaces_info_file << "2,1,1.0" << std::endl;

        // Ground station GSL interfaces info
        gsl_interfaces_info_file << "3,2,1.0" << std::endl;
        gsl_interfaces_info_file << "4,1,1.0" << std::endl;
        gsl_interfaces_info_file << "5,1,1.0" << std::endl;
        gsl_interfaces_info_file << "6,1,1.0" << std::endl;

        gsl_interfaces_info_file.close();

        // Dynamic state
        for (int64_t i = 0; i < simulation_end_time_ns; i += 100000000) {
            std::ofstream fstate_file;
            fstate_file.open (dyn_state_dir + "/fstate_" + std::to_string(i) + ".txt");
            if (i == 0) {
                fstate_file << "3,5,0,0,1" << std::endl;
                fstate_file << "0,5,1,0,0" << std::endl;
                fstate_file << "1,5,5,1,0" << std::endl;
                fstate_file << "3,6,1,1,1" << std::endl;
                fstate_file << "1,6,6,2,0" << std::endl;
                fstate_file << "4,5,1,0,1" << std::endl;
                fstate_file << "4,6,2,0,0" << std::endl;
                fstate_file << "2,6,6,0,0" << std::endl;
            }
//            } else if (i == 1600000000) {
//
//            } else if (i == 18700000000) {
//
//            }
            fstate_file.close();

            std::ofstream gsl_if_bandwidth_file;
            gsl_if_bandwidth_file.open (dyn_state_dir + "/gsl_if_bandwidth_" + std::to_string(i) + ".txt");
            gsl_if_bandwidth_file.close();
        }

        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);

        // Optimize TCP
        TcpOptimizer::OptimizeBasic(basicSimulation);

        // Read topology, and install routing arbiters
        Ptr<TopologySatelliteNetwork> topology = CreateObject<TopologySatelliteNetwork>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterSingleForwardHelper arbiterHelper(basicSimulation, topology->GetNodes());
        GslIfBandwidthHelper gslIfBandwidthHelper(basicSimulation, topology->GetNodes());

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology); // Requires enable_udp_burst_scheduler=true

        // Check all the accessors of the topology if it was interpreted correctly
        ASSERT_EQUAL(3, topology->GetNumSatellites());
        ASSERT_EQUAL(4, topology->GetNumGroundStations());
        ASSERT_EQUAL(7, topology->GetNodes().GetN());
        ASSERT_EQUAL(7, topology->GetNumNodes());
        NodeContainer satellite_nodes = topology->GetSatelliteNodes();
        ASSERT_EQUAL(3, satellite_nodes.GetN());
        for (size_t i = 0; i < satellite_nodes.GetN(); i++) {
            ASSERT_EQUAL(i, satellite_nodes.Get(i)->GetId());
        }
        NodeContainer ground_station_nodes = topology->GetGroundStationNodes();
        ASSERT_EQUAL(4, ground_station_nodes.GetN());
        for (size_t i = 0; i < ground_station_nodes.GetN(); i++) {
            ASSERT_EQUAL(3 + i, ground_station_nodes.Get(i)->GetId());
        }
        ASSERT_EXCEPTION(topology->IsSatelliteId(-1));
        ASSERT_EXCEPTION(topology->IsSatelliteId(7));
        ASSERT_EXCEPTION(topology->GetSatellite(3));
        for (size_t i = 0; i < 7; i++) {
            if (i < 3) {
                ASSERT_TRUE(topology->IsSatelliteId(i));
                ASSERT_FALSE(topology->IsGroundStationId(i));
                ASSERT_TRUE(topology->GetSatellite(i) != 0);
            } else {
                ASSERT_FALSE(topology->IsSatelliteId(i));
                ASSERT_TRUE(topology->IsGroundStationId(i));
                ASSERT_EQUAL(topology->NodeToGroundStationId(i), i - 3);
            }
        }
        ASSERT_EQUAL(3, topology->GetSatellites().size());
        ASSERT_EQUAL(4, topology->GetGroundStations().size());

        // TODO: Check network device components

        // Run simulation
        basicSimulation->Run();

        // Write UDP burst results
        udpBurstScheduler.WriteResults();

        // Read in UDP burst results
        std::vector<std::string> lines_incoming_csv = read_file_direct(temp_dir + "/logs_ns3/udp_bursts_incoming.csv");
        std::vector<double> incoming_rate_incl_headers_megabit_per_s;
        for (std::string line : lines_incoming_csv) {
            std::vector<std::string> line_spl = split_string(line, ",");
            std::cout << parse_positive_double(line_spl[6]) << std::endl;
            incoming_rate_incl_headers_megabit_per_s.push_back(parse_positive_double(line_spl[6]));
        }
        ASSERT_EQUAL_APPROX(incoming_rate_incl_headers_megabit_per_s.at(0), 4.0, 0.1);
        ASSERT_EQUAL_APPROX(incoming_rate_incl_headers_megabit_per_s.at(1), 10.0, 0.1);
        ASSERT_EQUAL_APPROX(incoming_rate_incl_headers_megabit_per_s.at(2), 6.0, 0.1);
        ASSERT_EQUAL_APPROX(incoming_rate_incl_headers_megabit_per_s.at(3), 4.0, 0.1);

        // Collect utilization statistics
        topology->CollectUtilizationStatistics();

        // Finalize the simulation
        basicSimulation->Finalize();

    }

};

////////////////////////////////////////////////////////////////////////////////////////
