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

class EndToEndTestCase : public TestCase {
public:
    EndToEndTestCase () : TestCase ("end-to-end") {};

    void DoRun () {

        std::string run_dir = "test_data/end_to_end/run";

        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(run_dir);

        // Optimize TCP
        TcpOptimizer::OptimizeBasic(basicSimulation);

        // Read topology, and install routing arbiters
        Ptr<TopologySatelliteNetwork> topology = CreateObject<TopologySatelliteNetwork>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterSingleForwardHelper arbiterHelper(basicSimulation, topology->GetNodes());
        GslIfBandwidthHelper gslIfBandwidthHelper(basicSimulation, topology->GetNodes());

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology); // Requires enable_udp_burst_scheduler=true

        // Run simulation
        basicSimulation->Run();

        // Write UDP burst results
        udpBurstScheduler.WriteResults();

        // Collect utilization statistics
        topology->CollectUtilizationStatistics();

        // Finalize the simulation
        basicSimulation->Finalize();

        // TODO: Check all UDP burst packets arrived
        // TODO: Check some timing one-ways?
        // TODO: Check some of the ISL utilizations?

        // Make sure these are removed
        remove_file_if_exists(run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/isl_utilization.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_burst_0_outgoing.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_burst_0_incoming.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        remove_dir_if_exists(run_dir + "/logs_ns3");

    }

};

////////////////////////////////////////////////////////////////////////////////////////
