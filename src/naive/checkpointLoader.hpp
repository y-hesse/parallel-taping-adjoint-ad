#ifndef ADJOINT_NAIVE_CHECKPOINTLOADER_HPP
#define ADJOINT_NAIVE_CHECKPOINTLOADER_HPP

#include <fstream>
#include <primal/primal.hpp>
#include <checkpoint.hpp>
#include <checkpointLoaderInterface.hpp>

namespace naive {
    class checkpointLoader;
    static const std::string mode = "Naive";
}

class naive::checkpointLoader : checkpointLoaderInterface {
private:
    std::vector<double> input;
public:
    checkpointLoader(int concurrent) {
    }

    void recordLoader(std::vector<double> &input) {
        this->input = input;
    }
    bool getCheckpoint(uint64_t i, checkpoint &c) {
        uint64_t to = i*windowThreadSize > size ? size : i*windowThreadSize;
        const checkpoint b(input, 0, to);
        PRIMAL::primal(b, [&i, &c] (const checkpoint &e) {
            if (i == 1) {
                c = e;
            }
            i--;
        });
        return true;
    }

};


#endif //ADJOINT_NAIVE_CHECKPOINTLOADER_HPP
