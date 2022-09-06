#ifndef ADJOINT_TEST_FUNCTION_HPP
#define ADJOINT_TEST_FUNCTION_HPP

#include <primal/primal.hpp>

namespace test_function {

    template<typename T>
    void f(std::vector<T> &x, T &u) {
        u = sin(x[1]);
        x[1] = pow(u, 3) + x[0];
    }

    template<typename T>
    void primal(std::vector<T> &inout);
    void primal(checkpoint c, dag* D);
    void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint);

}

#endif //ADJOINT_TEST_FUNCTION_HPP
