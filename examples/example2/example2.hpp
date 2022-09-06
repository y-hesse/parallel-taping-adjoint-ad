#ifndef ADJOINT_EXAMPLE2_HPP
#define ADJOINT_EXAMPLE2_HPP

#include <vector>
#include <checkpoint.hpp>
#include <functional>

namespace example2 {

    template<typename T>
    void primal(std::vector<T> &inout);
    void primal(checkpoint c, dag* D);
    void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint);

}


#endif //ADJOINT_EXAMPLE2_HPP
