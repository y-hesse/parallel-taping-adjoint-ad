#include "test_function.hpp"
#include <primal/primal.hpp>

namespace test_function {
    template<typename T>
    void primal(std::vector<T> &inout) {
        T u;
        for (uint64_t i = 0; i < size; ++i) {
            if (i == 0) {
                inout[1] = inout[0];
                continue;
            }
            f(inout, u);
        }
    }
    template void primal(std::vector<double> &inout);
    template void primal(std::vector<double_o> &inout);


    void primal(checkpoint c, dag* D) {
        std::vector<double_o> inout;
        uint64_t start;
        uint64_t end;
        c.start(D, inout, start, end);

        double_o u;
        for (uint64_t i = start; i < end; ++i) {
            if (i == 0) {
                inout[1] = inout[0];
                continue;
            }
            f(inout, u);
        }
    }


    void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint) {
        std::vector<double> inout;
        uint64_t start;
        uint64_t end;
        c.start(inout, start, end);

        std::cout << size << " a " << std::endl;;
        std::cout << windowThreadSize << " b "<< std::endl;

        double u;
        for (uint64_t i = start; i < end; ++i) {
            if (i % windowThreadSize == 0) {
                checkpoint a = checkpoint(inout, i);
                a.to = i + windowThreadSize > size ? size : i + windowThreadSize;
                addCheckpoint(a);
            }
            if (i == 0) {
                inout[1] = inout[0];
                continue;
            }
            f(inout, u);
        }
    }
}
