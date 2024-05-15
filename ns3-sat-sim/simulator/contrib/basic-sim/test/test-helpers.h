#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "ns3/test.h"

#define ASSERT_EQUAL(a, b) NS_TEST_ASSERT_MSG_EQ((a), (b), "")
#define ASSERT_NOT_EQUAL(a, b) NS_TEST_ASSERT_MSG_NE((a), (b), "")
#define ASSERT_TRUE(a) NS_TEST_ASSERT_MSG_EQ((a), true, "")
#define ASSERT_FALSE(a) NS_TEST_ASSERT_MSG_EQ((a), false, "")
#define ASSERT_EQUAL_APPROX(a, b, c) NS_TEST_ASSERT_MSG_EQ_TOL(a, b, c, "")
#define ASSERT_EXCEPTION(a) do { bool caught = false; try { a; } catch (std::exception& e) { caught = true; } ASSERT_TRUE(caught); } while(0)
#define ASSERT_PAIR_EQUAL(a, b) ASSERT_TRUE(((a) == (b)))

bool set_int64_contains(const std::set<int64_t>& s, const int64_t value) {
    return s.find(value) != s.end();
}

bool set_pair_int64_contains(const std::set<std::pair<int64_t, int64_t>>& s, const std::pair<int64_t, int64_t> value) {
    return s.find(value) != s.end();
}

#endif //TEST_HELPERS_H
