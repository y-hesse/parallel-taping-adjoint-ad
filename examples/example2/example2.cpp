#include "example2.hpp"
#include <primal/primal.hpp>

namespace example2 {

    template<typename T>
    void f(T &x1, T &x2, T &y) {
        T tmp = sin(x2);
        for (int i = 0; i < size; i++) {
            tmp = x1 + sin(tmp);
        }
        y = tmp*x2;
    }



    template<typename T>
    void primal(std::vector<T> &inout) {
        f(inout[0], inout[1], inout[0]);
    }
    template void primal(std::vector<double> &inout);
    template void primal(std::vector<double_o> &inout);


    void primal(checkpoint c, dag* D) {
        std::vector<double_o> inout;
        uint64_t start;
        uint64_t end;
        c.start(D, inout, start, end);


        for (uint64_t i = start; i < end; i++) {
            if (i == 0) {
                inout[2] = sin(inout[1]);
            }
            inout[2] = inout[0] + sin(inout[2]);
        }
        if (end == size) inout[0] = inout[2]*inout[1];

    }


    void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint) {
        std::vector<double> inout;
        uint64_t start;
        uint64_t end;
        c.start(inout, start, end);


        for (uint64_t i = start; i < end; i++) {
            if (i % windowThreadSize == 0) {
                checkpoint a = checkpoint(inout, i, i + windowThreadSize > size ? size : i + windowThreadSize);
                addCheckpoint(a);
            }
            if (i == 0) {
                inout[2] = sin(inout[1]);
            }
            inout[2] = inout[0] + sin(inout[2]);
        }
        if (end == size) inout[0] = inout[2]*inout[1];

    }
}
