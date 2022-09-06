#ifndef ADJOINT_DAG_TEST_HPP
#define ADJOINT_DAG_TEST_HPP

#include <dag.hpp>
#include <double_o.hpp>
#include "gtest/gtest.h"

/**
 * This test will build a dag by overloading a simple function and
 * then acquire a derivative.
 */

void f(std::vector<double_o> inout, dag* g) {
    for (int i = 0; i < inout.size(); ++i) {
        inout[i].registerInput(g);
    }

    inout[0] = sin(inout[0]);
    inout[1] = cos(inout[1]);
    inout[2] = inout[0]+inout[1];
    inout[3] = inout[2] * inout[2];
    inout[4] = inout[4] / inout[3];
    double_o u;
    u = inout[5];
    inout[5] = -inout[4] + u;

    for (int i = 0; i < inout.size(); ++i) {
        inout[i] = inout[i] + 1;
    }
}

TEST(DagTest, AllAroundTest) {
    std::vector<double_o> t(6, 1);
    dag* c = new dag();

    f(t, c);

    // this is a test
    uint64_t ram = c->getRam();

    EXPECT_EQ(ram, 8);

    /**
     * Test against hand calculated Jacobian
     */
    for (int i = 0; i < t.size(); ++i) {
        std::vector<double> adj(ram, 0);
        adj[i] = 1;
        c->interpret(adj);
        switch(i) {
            case 0:
                EXPECT_EQ(adj[i], cos(1));
                break;
            case 1:
                EXPECT_EQ(adj[i], -sin(1));
                break;
            case 2:
                EXPECT_EQ(adj[0], cos(1));
                EXPECT_EQ(adj[1], -sin(1));
                break;
            case 3:
                EXPECT_EQ(adj[0], 2*(sin(1)+cos(1))*cos(1));
                EXPECT_EQ(adj[1], 2*(sin(1)+cos(1))*(-sin(1)));
                break;
            case 4:
                EXPECT_EQ(adj[i], 1 / (pow(sin(1) + cos(1), 2)));
                break;
            case 5:
                EXPECT_EQ(adj[i], 1);
                break;
        }
    }

    delete c;
}



#endif //ADJOINT_DAG_TEST_HPP
