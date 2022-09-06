## Developer Guide

### Overview

- [Adjoint calculation (aad.hpp)](#adjoint-calculation)
- [Optimization / Understanding the profiler (profiler.hpp)](#optimization-understanding-the-profiler)
- [The DAG / DCG struct (dag.hpp)](#the-dag--dcg-struct)
- [Supporting custom checkpointLoaders](#supporting-custom-checkpointloaders)
- [Supporting more basic Operators (double_o)](#supporting-more-basic-operators)

### Adjoint Calculation

When calculating adjoints via overloading cpp routines there are three basic steps each
adjoint software needs to do.

Step1: Record the Tape, this is done by running the primal function with an overloaded
double datatype (double_o) this allows the dag to record dependencies between
variables and to store a dag that contains the derivatives accordingly.

Step2: Tape reversal, this was not the focus of this project so the reversal function is very
primitive. The adjoint vector is seeded with the user's input of the adjoint vector, after that
the graph is traversed in reverse order and the derivatives are joined using the chain rule of differentiation.

Step3: Harvesting the adjoints, the tape reversal provides a vector of adjoints.

Aligned with these three basic steps the aad.hpp provides a routine called aad. AAD takes 
an input vector for the primal and an adjoint vector that is later used to seed the tape reversal.

The focus of this project was to speed up tape recording by utilizing the parallel cpu architecture.
That is why the aad method can be run with different tape recording functions. More details about that can be found in the ["Supporting custom checkpointLoaders"](#supporting-custom-checkpointloaders) subsection.

The recording is split in two parts, recording the checkpoints and recording the dag.
As this problem is memory bound, there is no way to fit a complete dag of a compute intensive problem
into memory.
That is why the primal is segmented via checkpoints.

Using OpenMP multithreading Step1 and Step2 are combined. This results in tape recording and 
tape reversal being done simultaneously on different segments of the primal.

Using this we try to hide the overhead created by recording the tape compared to running the primal.

Be careful when doing changes to this routine as it is the heart of the programm that connects
all the separate parts.

### Optimization & Understanding the profiler

Running the software in profiler mode can be very useful to get a rough estimate of
the runtime the main aad routine will need. It is also useful when looking at parameters like the
*windowSize* as they can be increased depending on the primal and device you are running.

An example output of the profiler may look like:

```
OS: Windows 32-bit
Cores: 8
Mem: 17GB
Running primal took: 2544.8ms
Checkpoint size: 8064Byte
Overloading checkpoint took: 998ms
Overload-Factor: 31.3738x
Memory-Factor: 6.11289x
Best-Case time to finish: 86928.8ms
```
Next to the system information like the OS, the usable CPU cores / memory other 
parameters like the overloading factor are also of interest.

The overloading factor shows how much slower recording the tape is compared to running the
primal.

The memory factor shows the factor by which the *windowSize* can be multiplied to optimally use the available memory.

The best case ttf shows the theoretical best achievable time.

One other aspect that should be considered when wanting to improve performance significantly is looking at the
compiler and compiler optimization used. More information about this can be found in
the thesis performance analysis.

### The DAG / DCG struct

The tape data structure can be interpreted as a Directed Acyclic Graph (DAG) and is implemented as a
Directed Cyclic Graph (DCG).

The design is heavily inspired by Prof. Dr. Uwe Naumann's
research paper on "Reduction of the Random Access Memory Size in Adjoint Algorithmic Differentiation by Overloading"
[(https://arxiv.org/abs/2207.07018)](https://arxiv.org/abs/2207.07018)

### Supporting custom checkpointLoaders

As part of the thesis two distinct methods of generating and storing checkpoints 
where added

- Equidistant Disk Checkpointing
- In Memory Adaptive Checkpointing

How these two methods hold up is part of the thesis analysis.

When adding other custom loaders you should implement the abstract class checkpointLoaderInterface
and also orient yourself by looking at already implemented checkpointLoaders for example the 
memory::checkpointLoader.

```cpp
class checkpointLoader {
    /**
     * @param concurrent defines the CPU threads the checkpointLoader
     * is allowed to make use of.
     * 
     * Useful for debugging
     */
    checkpointLoader(int concurrent);

    /**
     * Called at the start of the programm, for initialization or complete recording
     */
    void recordLoader(std::vector<double> &input);
    
    /**
     * Each thread will request a chunk by providing the chunk id.
     */
    bool getCheckpoint(int64_t i, checkpoint &c);
}
```

### Supporting more basic Operators 

The double_o datatype is a drop in replacement for the cpp double.
However not all operators and other functions are supported.
The default supported operators include: {+, -, *, \, sin, cos, pow}

When adding your own adjust the double_o class in double_o.hpp.

Example: adding f = pow(a, b)

We need to know the derivative of our function in the directions of
all inputs, as pow only has one input this is simply f' = b*pow(a,b-1) 

```cpp
double_o pow(double_o in, int exponent);
```

this would be the function signature.

In the double_o.hpp file add

```cpp
friend double_o pow(const double_o &d1, const int exponent) {
    /**
     * The newly generated output
     */
    double_o res;
    
    /**
     * Test if the provided input is currently being recorded on a dag.
     */
    if (d1.g != nullptr) {
        // the output should also be recorded in that case.
        res.g = d1.g;
    
        // the derivative in direction of d1
        d1.record_arg(exponent * pow(d1.value, exponent-1));
        res.record_res(1);

    }
    // real operation
    res.value = pow(d1.value, exponent);
    return res;
}
```

It could also be helpful to look at how other operators are implemented to
get a good reference.