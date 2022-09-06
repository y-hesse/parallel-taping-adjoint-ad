#include "primal.hpp"

// used to define "global" variables

// The amount of CPU-Cores the System has provided this program
int cores = omp_get_max_threads();

// Total number or loop iterations / total number of possible checkpoints
// may need to be reduced if an uint64_t overflow is bound  to happen
uint64_t size = 30000;

// The amount of loop iterations that fit into the main memory
// must be smaller than the provided size
uint64_t windowSize = 1000;

// The amount of loop iterations which each CPU-Thread gets to process
uint64_t windowThreadSize = windowSize / (cores);

// The Total amount of chunks
uint64_t chunks = size % windowThreadSize == 0 ? (size / windowThreadSize) : (size / windowThreadSize) + 1;