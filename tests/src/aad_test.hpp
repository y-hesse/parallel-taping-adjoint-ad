#ifndef ADJOINT_AAD_TEST_HPP
#define ADJOINT_AAD_TEST_HPP

#include "test_function/test_function.hpp"
#include <aad.hpp>
#include "gtest/gtest.h"

TEST(AadTest, main) {
    std::vector<double> in = {1,0};
    std::vector<double> adj = {0,1};

    size = 10000;
    windowSize = 1000;
    recalculateValues();

    aad(in, adj);

    ASSERT_NEAR(adj[0], 0.552355, 0.001);
}

TEST(AadTest, main2) {
    std::vector<double> in = {1,0};
    std::vector<double> adj = {0,1};

    size = 16;
    windowSize = 8;
    recalculateValues();

    aad(in, adj);

    ASSERT_NEAR(adj[0], 0.391, 0.001);
}

#endif //ADJOINT_AAD_TEST_HPP
