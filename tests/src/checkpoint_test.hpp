#ifndef ADJOINT_CHECKPOINT_TEST_HPP
#define ADJOINT_CHECKPOINT_TEST_HPP

#include <checkpoint.hpp>
#include <double_o.hpp>
#include "gtest/gtest.h"

/**
 * Test all the Important API endpoints
 * of the checkpoint class
 * + writing
 * + reading
 * starting and creating
 */
TEST(CheckpointTest, SaveLoad) {
    std::vector<double_o> a(100, 10.);
    std::vector<double> b(100, 9.);

    checkpoint aC(a, 0, 99);
    checkpoint bC(b, 1, 100);

    dag* c = new dag();

    uint64_t from,to;
    aC.start(c, a, from, to);
    EXPECT_EQ(from, 0);
    EXPECT_EQ(to, 99);
    EXPECT_EQ(a[99].getValue(), 10.);

    aC.start(b, from, to);
    EXPECT_EQ(from, 0);
    EXPECT_EQ(to, 99);
    EXPECT_EQ(b[99], 10.);

    bC.start(b, from, to);
    EXPECT_EQ(from, 1);
    EXPECT_EQ(to, 100);
    EXPECT_EQ(b[99], 9.);

    delete c;
}

TEST(CheckpointTest, WriteRead) {
    std::vector<double> a(100, 10.),b;

    checkpoint aC(a, 0, 99);

    checkpoint bC;

    std::stringstream buf;

    buf << aC;
    buf >> bC;

    ASSERT_EQ(aC, bC);
}

#endif //ADJOINT_CHECKPOINT_TEST_HPP
