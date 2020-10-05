/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "end-to-end-test.h"
#include "manual-two-sat-two-gs-test.h"
#include "satellite-info-test.h"
#include "ground-station-info-test.h"
#include "end-to-end-special-test.h"

using namespace ns3;

class SatelliteNetworkTestSuite : public TestSuite {
public:
    SatelliteNetworkTestSuite() : TestSuite("satellite-network", UNIT) {

        // Running it complete with reading in files etc.
        AddTestCase(new EndToEndTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndSpecialTestCase, TestCase::QUICK);

        // Running it by creating every component manually (not using satellite-network.cc/h)
        AddTestCase(new ManualTwoSatTwoGsFirstTest, TestCase::QUICK);
        AddTestCase(new ManualTwoSatTwoGsDifferentPropSpeedTest, TestCase::QUICK);
        AddTestCase(new ManualTwoSatTwoGsUpSharedTest, TestCase::QUICK);
        AddTestCase(new ManualTwoSatTwoGsUpSharedUdpTest, TestCase::QUICK);
        AddTestCase(new ManualTwoSatTwoGsDownBothFullTest, TestCase::QUICK);
        AddTestCase(new ManualTwoSatTwoGsChangingForwardingTest, TestCase::QUICK);
        AddTestCase(new ManualTwoSatTwoGsChangingRateTest, TestCase::QUICK);

        // Simple info wrappers
        AddTestCase(new SatelliteInfoTestCase, TestCase::QUICK);
        AddTestCase(new GroundStationInfoTestCase, TestCase::QUICK);

    }
};
static SatelliteNetworkTestSuite SatelliteNetworkTestSuite;
