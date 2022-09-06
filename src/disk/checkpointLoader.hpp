#ifndef ADJOINT_DISK_CHECKPOINTLOADER_HPP
#define ADJOINT_DISK_CHECKPOINTLOADER_HPP

#include <fstream>
#include "primal/primal.hpp"
#include "checkpoint.hpp"
#include <checkpointLoaderInterface.hpp>
#include <chrono>
#include <mutex>
#include <string>
#include <condition_variable>

namespace disk {
    class checkpointLoader;
    static const std::string mode = "Disk";
}

class disk::checkpointLoader : checkpointLoaderInterface {
private:
    std::string id; // used to name the files to allow multi running of the program
    std::fstream is;
    std::vector<uint64_t> files;
    uint64_t currentLast = 0;
    std::mutex m;
    std::condition_variable cv;

    uint64_t recordCheckpoints(std::vector<double> &input);
    uint64_t recordCheckpoints(uint64_t from, std::vector<double> &input);
    uint64_t recordCheckpoints(uint64_t from, uint64_t to, std::vector<double> &input);

    void startStream() {
        if (!is.is_open()) {
            is.open("data/run-" + this->id + "-data" + std::to_string(files.back()) + ".ch");
            is.seekg(0, is.end);
            files.pop_back();
        }
    }
public:
    checkpointLoader(int concurrent) {
        auto start = std::chrono::system_clock::now();

        id = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count() % 1000000);
        files = std::vector<uint64_t>(concurrent);
    }

    void recordLoader(std::vector<double> &input);

    uint64_t getChecks() { return currentLast; }

    bool getCheckpoint(uint64_t i, checkpoint &c) {

        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [i, this] { return i == currentLast;});

        startStream();

        is.seekg(-1,std::ios_base::cur);
        while(is.peek() != '{') {
            is.seekg(-1,std::ios_base::cur);
            if (is.tellg() < 0) {
                if (files.empty()) {
                    return false;
                } else {
                    is.close();
                    startStream();
                }
            }
        }
        auto tmp = is.tellg();
        is >> c;
        is.seekg(tmp);

        currentLast--;
        cv.notify_all();

        return true;
    }

};


#endif //ADJOINT_DISK_CHECKPOINTLOADER_HPP
