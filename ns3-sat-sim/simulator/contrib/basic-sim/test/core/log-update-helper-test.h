/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "../test-helpers.h"
#include "ns3/log-update-helper.h"

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class LogUpdateHelperValidTestCase : public TestCase {
public:
    LogUpdateHelperValidTestCase() : TestCase("log-update-helper valid") {};

    void DoRun() {
        std::vector<std::tuple<int64_t, int64_t, int64_t>> result;
        LogUpdateHelper logUpdateHelper;

        // Empty end at t = 0
        logUpdateHelper = LogUpdateHelper();
        result = logUpdateHelper.Finalize(0);
        ASSERT_EQUAL(result.size(), 0);

        // Empty end at t = not 0
        logUpdateHelper = LogUpdateHelper();
        result = logUpdateHelper.Finalize(100);
        ASSERT_EQUAL(result.size(), 0);

        // One entry, but it finalizes at the moment
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(0, 9);
        result = logUpdateHelper.Finalize(0);
        ASSERT_EQUAL(result.size(), 0);

        // One entry, it finalizes sometime later
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(0, 9);
        result = logUpdateHelper.Finalize(77);
        ASSERT_EQUAL(result.size(), 1);
        ASSERT_EQUAL(std::get<0>(result[0]), 0);
        ASSERT_EQUAL(std::get<1>(result[0]), 77);
        ASSERT_EQUAL(std::get<2>(result[0]), 9);

        // One entry, it finalizes one tick later
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(0, -1);
        result = logUpdateHelper.Finalize(1);
        ASSERT_EQUAL(result.size(), 1);
        ASSERT_EQUAL(std::get<0>(result[0]), 0);
        ASSERT_EQUAL(std::get<1>(result[0]), 1);
        ASSERT_EQUAL(std::get<2>(result[0]), -1);

        // One entry later, it finalizes sometime later
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, -100);
        result = logUpdateHelper.Finalize(7);
        ASSERT_EQUAL(result.size(), 1);
        ASSERT_EQUAL(std::get<0>(result[0]), 3);
        ASSERT_EQUAL(std::get<1>(result[0]), 7);
        ASSERT_EQUAL(std::get<2>(result[0]), -100);

        // One entry later, gets updated a bunch, gets merged, it finalizes sometime later
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, -100);
        logUpdateHelper.Update(3, -100);
        logUpdateHelper.Update(5, -100);
        result = logUpdateHelper.Finalize(7);
        ASSERT_EQUAL(result.size(), 1);
        ASSERT_EQUAL(std::get<0>(result[0]), 3);
        ASSERT_EQUAL(std::get<1>(result[0]), 7);
        ASSERT_EQUAL(std::get<2>(result[0]), -100);

        // One entry later, gets updated a bunch, last value is different but at finalization moment
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, -100);
        logUpdateHelper.Update(3, -100);
        logUpdateHelper.Update(7, -99);
        result = logUpdateHelper.Finalize(7);
        ASSERT_EQUAL(result.size(), 1);
        ASSERT_EQUAL(std::get<0>(result[0]), 3);
        ASSERT_EQUAL(std::get<1>(result[0]), 7);
        ASSERT_EQUAL(std::get<2>(result[0]), -100);

        // Two entries, last value is different but at finalization moment
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, -100);
        logUpdateHelper.Update(5, 87);
        logUpdateHelper.Update(7, -99);
        result = logUpdateHelper.Finalize(7);
        ASSERT_EQUAL(result.size(), 2);
        ASSERT_EQUAL(std::get<0>(result[0]), 3);
        ASSERT_EQUAL(std::get<1>(result[0]), 5);
        ASSERT_EQUAL(std::get<2>(result[0]), -100);
        ASSERT_EQUAL(std::get<0>(result[1]), 5);
        ASSERT_EQUAL(std::get<1>(result[1]), 7);
        ASSERT_EQUAL(std::get<2>(result[1]), 87);

        // Many entries
        logUpdateHelper = LogUpdateHelper();
        for (int i = 0; i < 100; i++) {
            logUpdateHelper.Update(i, i * 2);
        }
        result = logUpdateHelper.Finalize(900);
        ASSERT_EQUAL(result.size(), 100);
        for (int i = 0; i < 100; i++) {
            ASSERT_EQUAL(std::get<0>(result[i]), i);
            ASSERT_EQUAL(std::get<1>(result[i]), i == 99 ? 900 : (i + 1));
            ASSERT_EQUAL(std::get<2>(result[i]), i * 2);
        }

        // Many entries with overwrite
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, -100);
        logUpdateHelper.Update(5, 87);
        logUpdateHelper.Update(5, 33);
        logUpdateHelper.Update(5, 44);
        result = logUpdateHelper.Finalize(7);
        ASSERT_EQUAL(result.size(), 2);
        ASSERT_EQUAL(std::get<0>(result[0]), 3);
        ASSERT_EQUAL(std::get<1>(result[0]), 5);
        ASSERT_EQUAL(std::get<2>(result[0]), -100);
        ASSERT_EQUAL(std::get<0>(result[1]), 5);
        ASSERT_EQUAL(std::get<1>(result[1]), 7);
        ASSERT_EQUAL(std::get<2>(result[1]), 44);

        // Many entries with merge
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, 1234567);
        logUpdateHelper.Update(5, 1234567);
        logUpdateHelper.Update(7, 7573);
        logUpdateHelper.Update(7, 1234567);
        result = logUpdateHelper.Finalize(9);
        ASSERT_EQUAL(result.size(), 1);
        ASSERT_EQUAL(std::get<0>(result[0]), 3);
        ASSERT_EQUAL(std::get<1>(result[0]), 9);
        ASSERT_EQUAL(std::get<2>(result[0]), 1234567);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class LogUpdateHelperInvalidTestCase : public TestCase {
public:
    LogUpdateHelperInvalidTestCase() : TestCase("log-update-helper invalid") {};

    void DoRun() {
        std::vector<std::tuple<int64_t, int64_t, int64_t>> result;
        LogUpdateHelper logUpdateHelper;

        // Update negative time
        logUpdateHelper = LogUpdateHelper();
        ASSERT_EXCEPTION(logUpdateHelper.Update(-1, 4));

        // Update time in the past
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, 100);
        logUpdateHelper.Update(6, 788);
        logUpdateHelper.Update(6, 33);
        logUpdateHelper.Update(8, 22);
        ASSERT_EXCEPTION(logUpdateHelper.Update(7, 22));

        // Finalize negative time
        logUpdateHelper = LogUpdateHelper();
        ASSERT_EXCEPTION(logUpdateHelper.Finalize(-99));

        // Finalize time in the past
        logUpdateHelper = LogUpdateHelper();
        logUpdateHelper.Update(3, 100);
        logUpdateHelper.Update(6, 788);
        logUpdateHelper.Update(6, 33);
        logUpdateHelper.Update(8, 22);
        ASSERT_EXCEPTION(logUpdateHelper.Finalize(7));

    }
};

////////////////////////////////////////////////////////////////////////////////////////
