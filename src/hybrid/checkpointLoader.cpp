#include <vector>
#include "checkpointLoader.hpp"
#include <primal/primal.hpp>

using namespace hybrid;

checkpoint hybrid::checkpointLoader::recordMem(uint64_t from, uint64_t to, std::vector<double> &input) {

    checkpoint start = checkpoint(input, 0);

    if (checks.size() != 0) {
        start = checks.back();
    }

    if (start.to >= to) {
        checks.pop_back();
        return start;
    }

    start.to = to;

    uint64_t half = (from + to) / 2;

    PRIMAL::primal(start, [&start, &half, this, &to] (const checkpoint &c) {
        if (c.from >= half && c.to < to && this->checks.size() < this->memory) {
            this->checks.push_back(c);
            half = (c.to + to) / 2;
        }
        if (c.to >= to) {
            start = c;
        }
    });

    return start;
}

void hybrid::checkpointLoader::recordDisk(checkpoint &start) {

    uint64_t from = start.from;

    checkpoint pre = start;

    std::ofstream file("data/data" + std::to_string(from) + ".ch");
    PRIMAL::primal(pre, [&file] (const checkpoint &c) {
        file << c;
    });
    file.close();
}
