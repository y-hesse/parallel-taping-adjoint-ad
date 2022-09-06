#include <cstdlib>
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include "checkpoint.hpp"
using namespace std;

#include "burgers.h"
#include "primal/primal.hpp"

namespace burgersFunction {

    template<typename T>
    void primal(std::vector<T> &inout) {
        const double d=1e-3;
        burgers(size,d,inout);
    }
    template void primal(std::vector<double> &inout);
    template void primal(std::vector<double_o> &inout);

    void primal(checkpoint c, dag *D) {
        std::vector<double_o> inout;
        uint64_t start;
        uint64_t end;
        c.start(D, inout, start, end);

        const double d=1e-3;
        int n=inout.size();
        vector<double_o> A((n-2)*3+4,0), r(n,0), r_t(n), y_t(n), y_prev(n);
        double_o diffusion_t;
        double_o advection;
        double_o advection_t;
        for (int j = start; j < end; j++) {
            for (int i = 0; i < n; ++i) {
                y_prev[i] = inout[i];
                r[i] = 0;
            }
            for (int i = 0; i < (n-2)*3+4; ++i) {
                A[i] = 0;
            }
            newton(size, d, y_prev, inout, A, r, y_t, r_t, diffusion_t, advection, advection_t);
        }
    }


    void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint) {
        std::vector<double> inout;
        uint64_t start;
        uint64_t end;
        c.start(inout, start, end);



        const double d=1e-3;

        int n=inout.size();
        vector<double> A((n-2)*3+4,0), r(n,0), r_t(n), y_t(n), y_prev(n);
        double diffusion_t;
        double advection;
        double advection_t;
        for (uint64_t j = start; j < end; j++) {

            if (j % windowThreadSize == 0) {
                checkpoint a = checkpoint(inout, j);
                a.to = j + windowThreadSize > size ? size : j + windowThreadSize;
                addCheckpoint(a);
            }

            for (int i = 0; i < n; ++i) {
                y_prev[i] = inout[i];
                r[i] = 0;
            }
            for (int i = 0; i < (n-2)*3+4; ++i) {
                A[i] = 0;
            }

            newton(size, d, y_prev, inout, A, r, y_t, r_t, diffusion_t, advection, advection_t);

        }
    }
}