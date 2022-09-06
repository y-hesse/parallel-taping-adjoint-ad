#ifndef ADJOINT_DOUBLE_O_TEST_HPP
#define ADJOINT_DOUBLE_O_TEST_HPP

#include <double_o.hpp>
#include "gtest/gtest.h"

TEST(DoubleOTest, MathmaticsTest) {
    double_o x = 5;
    double_o y = 2;
    double_o z = 7;
    ASSERT_EQ((x+y).getValue(), z.getValue());
    ASSERT_EQ((x*y).getValue(), 10);
    ASSERT_EQ((cos(sin(x))).getValue(), cos(sin(5)));
}


#endif //ADJOINT_DOUBLE_O_TEST_HPP
