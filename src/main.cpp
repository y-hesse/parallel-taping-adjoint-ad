#include <vector>
#include <aad.hpp>
#include <profiler.hpp>

int main(int argc, char** argv) {
    /*
     * Initialize the Input Vector
     */
    std::vector<double> in(303, 1);
    for (int i=1;i<303-1;i++) in[i]=sin((2*pi*i)/3);

    //std::vector<double> in = {1,1,0};
    /*
     * Initialize the output vector
     * If the Vector is to small it will be autofilled with 0's
     */
    //std::vector<std::vector<double>> adjoints = {{0, 1}, {0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 1}};
    std::vector<double> adjoints = {0, 1};

    /*
     * Run a Profiling run to get rough estimates about runtime
     */
    profile(in, adjoints);

    /*
     * Calculate the Adjoints
     */
    int loops = 1;
    uint64_t time = 0;
    for (int i = 0; i < loops; ++i) {
        std::vector<double> c = in;
        auto adj_tmp = adjoints;
        uint64_t d = 0;
        aad(c, adj_tmp, d);
        time += d;
        std::cout << "Run " << i+1 << "/" << loops << ": " << d << " (~" << time/(i+1) << ")";
    }
    return 0;
}