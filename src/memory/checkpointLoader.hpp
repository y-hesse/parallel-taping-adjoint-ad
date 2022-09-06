#ifndef ADJOINT_MEMORY_CHECKPOINTLOADER_HPP
#define ADJOINT_MEMORY_CHECKPOINTLOADER_HPP

#include <fstream>
#include "primal/primal.hpp"
#include "checkpoint.hpp"
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <checkpointLoaderInterface.hpp>

namespace memory {
    class checkpointLoader;
    static const std::string mode = "Memory";
}

class memory::checkpointLoader : checkpointLoaderInterface {
private:
    int memory = 1000-1;
    int currentLast = chunks;
    std::vector<checkpoint> checks;
    std::mutex m;
    std::condition_variable cv;
    std::vector<double> in;
public:
    checkpointLoader(int concurrent) {
        checks = std::vector<checkpoint>();
        checks.reserve(memory);
    }

    void recordLoader(std::vector<double> &input) override {
        in = input;
    }

    checkpoint recordCheckpoints(uint64_t from, uint64_t to, std::vector<double> &input);

    bool getCheckpoint(uint64_t i, checkpoint &c) override {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [i, this] { return i == currentLast;});

        uint64_t to = (i)*windowThreadSize > size ? size : (i)*windowThreadSize;

        c = recordCheckpoints(0, to, in);
        currentLast--;
        cv.notify_all();
        return true;
    }

};


#endif //ADJOINT_MEMORY_CHECKPOINTLOADER_HPP
