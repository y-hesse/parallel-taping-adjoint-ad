#ifndef ADJOINT_PRIMAL_CPP
#define ADJOINT_PRIMAL_CPP

/**
 * Define the PRIMAL namespace that should be used
 */
#ifndef PRIMAL
#define PRIMAL burgersFunction
#endif

#ifndef CHECKPOINTING
#define CHECKPOINTING disk
#endif

#ifndef DEBUG
#define DEBUG "Verbose"
#endif

#include "checkpoint.hpp"
#include "dag.hpp"
#include "omp.h"
#include <functional>
#include <../examples/burgers/burgers.h>
#include <../examples/example1/example1.hpp>
#include <../examples/example2/example2.hpp>

extern int cores;
extern uint64_t size,windowSize,windowThreadSize,chunks;

/**
 * If changes where made to the windowSize or size they have to be propagated
 */
static void recalculateValues() {
// The amount of loop iterations which each CPU-Thread gets to process
    windowThreadSize = windowSize / (cores);

// The Total amount of chunks
    chunks = size % windowThreadSize == 0 ? (size / windowThreadSize) : (size / windowThreadSize) + 1;
}

#endif //ADJOINT_PRIMAL_CPP
