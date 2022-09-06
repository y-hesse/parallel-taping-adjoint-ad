## User Guide

This software's intended use is to calculate adjoints of algorithmically differentiable C++ routines


This is done by overloading the algorithmic operators such as {+, -, *, /, sin, cos}.
This operator list can be easily extended ([check the developer guide](DEVELOPER_GUIDE.md)).

Any C++ routine provided needs to be adjusted and created in a separate namespace.
Your routine needs to be rewritten to support three different
function signatures.

```cpp
template<typename T>
void primal(std::vector<T> &inout);
void primal(checkpoint c, dag* D);
void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint);
```

#### The Checkpoint class:

The Checkpoint class provides an API to segment your function.
These checkpoints are then used to overload different "areas" of your function
concurrently.
To make use of this efficiently you need to have a rough estimate
of the runtime and loop iterations your function will need.
You also need to be aware of the memory or assisting variables
your function needs. These need to be identified and in the worst
cased stored in the checkpoints. More details on this step will follow.


#### The double_o class:

This class represents the overloaded double datatype
it provides *most* functionality a double provides
and should be used as a drop in replacement for any
double's in your code.

##### Example (original):

```cpp
template<typename T>
void primal(std::vector<T> &input_output) {
    T u;
    for (int i = 1; i < 100; ++i) {
        T u;
        u = sin(x[i-1]);
        x[i] = u*u + x[0];
    }
}
```

##### Example (transformed):

```cpp
namespace myPrimal {
    template<typename T>
    void primal(std::vector<T> &input_output) {
        for (int i = 1; i < 100; ++i) {
            T u;
            u = sin(input_output[i-1]);
            input_output[i] = u*u + input_output[0];
        }
    }
    
    void primal(checkpoint c, dag* D) {
        std::vector<double_o> input_output;
        uint64_t from; to;
        
        // Aquire all the values from the checkpoint
        // and register the inputs on the Graph
        c.start(D, input_output, from, to);
        
        // It is important to reuse temporary variables by
        // pulling them out of the loop instead of reinitializing them
        // in each iteration.
        T u;
        for (uint64_t i = from; i < to; ++i) {
            u = sin(input_output[i-1]);
            input_output[i] = u*u + input_output[0];
        }
    }
    
    void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint) {
        std::vector<double_o> input_output;
        uint64_t from; to;
        
        // Aquire all the values from the checkpoint
        c.start(input_output, from, to);
        
        // It is important to reuse temporary variables by
        // pulling them out of the loop instead of reinitializing them
        // in each iteration.
        T u;
        for (uint64_t i = from; i < to; ++i) {
            for (i % windowThreadedSize == 0) {
                checkpoint tmp_check(input_output, from, from+windowThreadedSize);
                addCheckpoint(tmp_check);
            }
            u = sin(input_output[i-1]);
            input_output[i] = u*u + input_output[0];
        }
    }
}
```

Now to embed the newly transformed primal function into the code we need to add
it to be compiled in the cmake file.

And to be used by the software we need to edit primal.hpp
and set the size and define our PRIMAL namespace
```cpp
#define PRIMAL myPrimal 
...
static uint64_t size = 100;
```

Global external parameters that can be useful when deciding on where to make a cut in your function are:
- size
- windowThreadedSize

<ins>Size</ins> represents the total loop iterations your function will do

<ins>WindowThreadedSize</ins> is the maximal size of consecutive loop iterations that fit into your local machines memory
when overloading the function.

To then calculate the adjoints simply create a main.cpp file that looks like this
````cpp
#include <aad.hpp>
#include <profiler.hpp>
int main() {
    /*
     * Initialize the Input Vector
     */
    std::vector<double> in(100, 1);

    /*
     * Initialize the output vector
     * If the Vector is to small it will be autofilled with 0's
     */
    std::vector<double> adjoints = {0,1};

    /*
     * Calculate the Adjoints
     */
    aad(in, adjoints);
    return 0;
} 
````