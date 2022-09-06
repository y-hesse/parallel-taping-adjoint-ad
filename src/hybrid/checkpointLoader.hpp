#ifndef ADJOINT_HYBRID_CHECKPOINTLOADER_HPP
#define ADJOINT_HYBRID_CHECKPOINTLOADER_HPP

#include <fstream>
#include "primal/primal.hpp"
#include "checkpoint.hpp"
#include "checkpointLoaderInterface.hpp"
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace hybrid {
    class checkpointLoader;
    static const std::string mode = "Hybrid";
}

class hybrid::checkpointLoader : checkpointLoaderInterface {
private:
    int memory = 10-1;
    int currentLast = chunks;
    std::vector<checkpoint> checks;
    std::vector<uint64_t> files;
    std::mutex m;
    std::condition_variable cv;
    std::vector<double> in;
    std::fstream is;
    void startStream() {
        if (!is.is_open()) {
            is.open("data/data" + std::to_string(files.back()) + ".ch");
            is.seekg(0, is.end);
            files.pop_back();
        }
    }

public:
    checkpointLoader(int concurrent) {
        checks = std::vector<checkpoint>();
        checks.reserve(memory);
    }

    void recordLoader(std::vector<double> &input) {
        in = input;
        checkpoint c = recordMem(0, size, in);

        for (int i = 0; i < checks.size(); ++i) {
            if (i == checks.size()-1) {
                checks[i].to = c.from;
            } else {
                checks[i].to = checks[i + 1].from;
            }
        }
        checks.push_back(c);
        files = std::vector<uint64_t>(checks.size());
    }

    checkpoint recordMem(uint64_t from, uint64_t to, std::vector<double> &input);
    void recordDisk(checkpoint &start);

    bool getCheckpoint(uint64_t i, checkpoint &c) {
        // do the last recording
        int64_t g = (int64_t)i - (chunks-checks.size());
        if (i != chunks && (i) >= chunks-checks.size()) {
            g = i - (chunks-checks.size());
            if (g == 0) {
                checkpoint st(in, 0);
                st.to = checks[0].from;
                files[0] = 0;
                recordDisk(st);
            } else {
                files[g] = checks[g-1].from;
                recordDisk(checks[g-1]);
            }
        }

        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [i, this] { return i == currentLast; });

        // The very first Thread
        if (i == chunks) {
            c = checks.back();
            currentLast--;
            cv.notify_all();
            return true;
        } else {
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
    }
};


#endif //ADJOINT_HYBRID_CHECKPOINTLOADER_HPP
