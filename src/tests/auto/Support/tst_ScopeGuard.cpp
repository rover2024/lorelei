// SPDX-License-Identifier: MIT

#include <utility>

#include <lorelei/Support/ScopeGuard.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

BOOST_AUTO_TEST_SUITE(test_ScopeGuard)

BOOST_AUTO_TEST_CASE(runs_action_on_scope_exit) {
    int n = 0;
    {
        auto guard = makeScopeGuard([&] { n++; });
        BOOST_TEST(n == 0); // not yet
    }
    BOOST_TEST(n == 1); // fired on destruction
}

BOOST_AUTO_TEST_CASE(dismiss_cancels_the_action) {
    int n = 0;
    {
        auto guard = makeScopeGuard([&] { n++; });
        guard.dismiss();
    }
    BOOST_TEST(n == 0);
}

BOOST_AUTO_TEST_CASE(move_transfers_ownership_once) {
    int n = 0;
    {
        auto guard = makeScopeGuard([&] { n++; });
        auto moved = std::move(guard); // the moved-from guard must not fire
    }
    BOOST_TEST(n == 1);
}

BOOST_AUTO_TEST_SUITE_END()
