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

class SatelliteInfoTestCase : public TestCase {
public:
    SatelliteInfoTestCase () : TestCase ("satellite-info") {};

    void DoRun () {

        Satellite satellite = Satellite();
        satellite.SetName("ISS (ZARYA)");
        satellite.SetTleInfo(
                "1 25544U 98067A   20274.52061263  .00002472  00000-0  53335-4 0  9998",
                "2 25544  51.6445 187.2657 0001412 107.7286  78.8629 15.48811122248346"
        );
        ASSERT_EQUAL(satellite.GetName(), "ISS (ZARYA)");

    }

};

////////////////////////////////////////////////////////////////////////////////////////
