/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "tcp-flow-schedule-reader-test.h"
#include "udp-burst-schedule-reader-test.h"
#include "tcp-flow-end-to-end-test.h"
#include "pingmesh-end-to-end-test.h"
#include "manual-end-to-end-test.h"
#include "udp-burst-end-to-end-test.h"

using namespace ns3;

class BasicAppsTestSuite : public TestSuite {
public:
    BasicAppsTestSuite() : TestSuite("basic-sim-apps", UNIT) {

        // TCP flow schedule reader
        AddTestCase(new TcpFlowScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowScheduleReaderInvalidTestCase, TestCase::QUICK);

        // UDP burst schedule reader
        AddTestCase(new UdpBurstScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstScheduleReaderInvalidTestCase, TestCase::QUICK);

        // Manual end-to-end, which means the application helpers are used
        // directly instead of the schedulers reading from files
        AddTestCase(new ManualEndToEndTestCase, TestCase::QUICK);

        // TCP flows end-to-end
        AddTestCase(new TcpFlowEndToEndOneToOneEqualStartTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndOneToOneSimpleStartTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndOneToOneApartStartTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndEcmpSimpleTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndEcmpRemainTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndNonExistentRunDirTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndOneDropOneNotTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndConnFailTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndPrematureCloseTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndNotEnabledTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndInvalidLoggingIdTestCase, TestCase::QUICK);

        // Pingmesh end-to-end
        AddTestCase(new PingmeshEndToEndNineAllTestCase, TestCase::QUICK);
        AddTestCase(new PingmeshEndToEndFivePairsTestCase, TestCase::QUICK);
        AddTestCase(new PingmeshEndToEndCompetitionTcpTestCase, TestCase::QUICK);
        AddTestCase(new PingmeshEndToEndNotEnabledTestCase, TestCase::QUICK);

        // UDP burst end-to-end
        AddTestCase(new UdpBurstEndToEndOneToOneEqualStartTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndSingleOverflowTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndDoubleEnoughTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndDoubleOverflowTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndNotEnabledTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndInvalidLoggingIdTestCase, TestCase::QUICK);

    }
};
static BasicAppsTestSuite basicAppsTestSuite;
