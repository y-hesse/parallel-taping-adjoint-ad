#include "checkpointLoader.hpp"
#include <vector>
#include <primal/primal.hpp>

void disk::checkpointLoader::recordLoader(std::vector<double> &input) {
    uint64_t setSize = size/(uint64_t)files.size();
    uint64_t b = 0;
#pragma omp parallel for default(none) shared(files, setSize, b, input)
    for (int i = 0; i < files.size(); i++) {
        uint64_t c;
        files[i] = setSize*i;
        if (i == files.size()-1) {
            c = recordCheckpoints(setSize*i, input);
        } else {
            c = recordCheckpoints(setSize*i, (setSize*(i+1))-1, input);
        }
#pragma omp critical
        {
            b += c;
        };
    }

    currentLast = b;
}

uint64_t disk::checkpointLoader::recordCheckpoints(uint64_t from, uint64_t to, std::vector<double> &input) {
    // start with blank checkpoint
    checkpoint pre(input, 0);

    uint64_t count = 0;

    if (from != 0) {
        pre.to = from;

        PRIMAL::primal(pre, [&pre] (const checkpoint &c) {
            pre = c;
        });
    }

    pre.to = to;

    std::ofstream file("data/run-" + this->id + "-data" + std::to_string(from) + ".ch");
    PRIMAL::primal(pre, [&file, &count, &to] (const checkpoint &c) {
        if (c.to <= to) {
            file << c;
            count++;
        }
    });
    file.close();
    return count;
}

uint64_t disk::checkpointLoader::recordCheckpoints(uint64_t from, std::vector<double> &input) {
    return checkpointLoader::recordCheckpoints(from, size, input);
}

uint64_t disk::checkpointLoader::recordCheckpoints(std::vector<double> &input) {
    return checkpointLoader::recordCheckpoints(0, size, input);
}