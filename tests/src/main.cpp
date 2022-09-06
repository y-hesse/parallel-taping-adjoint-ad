#define CHECKPOINTING naive
#define DEBUG "Timing"
#define PRIMAL test_function

#include <gtest/gtest.h>
#include "double_o_test.hpp"
#include "dag_test.hpp"
#include "checkpoint_test.hpp"
#include "aad_test.hpp"


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

    return 0;
}