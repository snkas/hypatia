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

class GroundStationInfoTestCase : public TestCase {
public:
    GroundStationInfoTestCase () : TestCase ("ground-station-info") {};

    void DoRun () {

        GroundStation gs = GroundStation(
                123,
                "Test",
                3.8,
                1.12121,
                90.1,
                Vector(10.001, 8.132, 123214.997)
        );
        ASSERT_EQUAL(gs.GetGid(), 123);
        ASSERT_EQUAL(gs.GetName(), "Test");
        ASSERT_EQUAL(gs.GetLatitude(), 3.8);
        ASSERT_EQUAL(gs.GetLongitude(), 1.12121);
        ASSERT_EQUAL(gs.GetElevation(), 90.1);
        ASSERT_EQUAL(gs.GetCartesianPosition().x, 10.001);
        ASSERT_EQUAL(gs.GetCartesianPosition().y, 8.132);
        ASSERT_EQUAL(gs.GetCartesianPosition().z, 123214.997);
        ASSERT_EQUAL(gs.ToString(), "Ground station[gid=123, name=Test, latitude=3.8, longitude=1.12121, elevation=90.1, cartesian_position=10.001:8.132:123215]");

    }

};

////////////////////////////////////////////////////////////////////////////////////////
