/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "end-to-end-special-test.h"
#include "end-to-end-test.h"
#include "ground-station-info-test.h"
#include "manual-two-sat-two-gs-test.h"
#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "satellite-info-test.h"

using namespace ns3;

class SatelliteNetworkTestSuite : public TestSuite {
public:
  SatelliteNetworkTestSuite() : TestSuite("satellite-network", Type::UNIT) {

    // Running it complete with reading in files etc.
    AddTestCase(new EndToEndTestCase, TestCase::Duration::QUICK);
    AddTestCase(new EndToEndSpecialTestCase, TestCase::Duration::QUICK);

    // Running it by creating every component manually (not using
    // satellite-network.cc/h)
    AddTestCase(new ManualTwoSatTwoGsFirstTest, TestCase::Duration::QUICK);
    AddTestCase(new ManualTwoSatTwoGsDifferentPropSpeedTest,
                TestCase::Duration::QUICK);
    AddTestCase(new ManualTwoSatTwoGsUpSharedTest, TestCase::Duration::QUICK);
    AddTestCase(new ManualTwoSatTwoGsUpSharedUdpTest,
                TestCase::Duration::QUICK);
    AddTestCase(new ManualTwoSatTwoGsDownBothFullTest,
                TestCase::Duration::QUICK);
    AddTestCase(new ManualTwoSatTwoGsChangingForwardingTest,
                TestCase::Duration::QUICK);
    AddTestCase(new ManualTwoSatTwoGsChangingRateTest,
                TestCase::Duration::QUICK);

    // Simple info wrappers
    AddTestCase(new SatelliteInfoTestCase, TestCase::Duration::QUICK);
    AddTestCase(new GroundStationInfoTestCase, TestCase::Duration::QUICK);
  }
};
static SatelliteNetworkTestSuite SatelliteNetworkTestSuite;
