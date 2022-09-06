#ifndef ADJOINT_PROFILER_HPP
#define ADJOINT_PROFILER_HPP

#include <primal/primal.hpp>

using namespace PRIMAL;

#ifdef _WIN64
#include <windows.h>
#include <chrono>

unsigned long long getTotalSystemMemory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}
#elif __unix || __unix__
#include <unistd.h>

unsigned long long getTotalSystemMemory()
{
    // currently no reliable way of finding the max memory on linux
    return (unsigned long long)144*1000*1000*1000;
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}
#endif

std::string getOsName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
    #elif __APPLE__ || __MACH__
    return "Mac OSX";
    #elif __linux__
    return "Linux";
    #elif __FreeBSD__
    return "FreeBSD";
    #elif __unix || __unix__
    return "Unix";
    #else
    return "Other";
#endif
}

void profile(std::vector<double> input, std::vector<double> adjoints) {
    // Get Available System Data
    // OS
    std::cout << "OS: " << getOsName() << std::endl;
    // Cores
    std::cout << "Cores: " << cores << std::endl;

    // Memory
    std::cout << "Mem: " << getTotalSystemMemory() / 1000 / 1000 / 1000 << "GB" << std::endl;

    // Run Primal
    long long totalTime = 0;
#pragma omp parallel for default (none) shared(input, totalTime)
    for (int i = 0; i < 5; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto in_copy = input;
        primal(in_copy);
        auto stop = std::chrono::high_resolution_clock::now();
#pragma omp critical
        totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    }
    totalTime = totalTime / 5;
    std::cout << "Running Primal took: " << totalTime << "ms" << std::endl;


    // Run Primal Overloaded 1 Checkpoint
    checkpoint g(input, 0, windowThreadSize);

    // 1 Checkpoint Size
    std::cout << "Checkpoint Size: " << sizeof(g) + g.inputs.size()*sizeof(double) << "Byte" << std::endl;


    // Overload Ratio
    auto start = std::chrono::high_resolution_clock::now();
    dag* a = new dag();
    primal(g, a);
    auto stop = std::chrono::high_resolution_clock::now();
    auto oneChunk = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "CheckpointTapeTime: " << oneChunk << "ms" << std::endl;

    std::cout << "Overload Factor: " << ((double)oneChunk*chunks) / (double)totalTime << "x" << std::endl;
    // Maximum Checkpoints in Memory
    uint64_t realSize = a->getMemorySize();
    uint64_t memory = realSize*cores;
    std::string unit = "GB";
    if(memory/1000/1000/1000 < 10) {
        memory = memory/1000/1000;
        unit = "MB";
    } else {
        memory = memory/1000/1000/1000;
    }
    std::cout << "Used Memory: ~" << memory <<  unit << "(" << ((double)getTotalSystemMemory()/(double)(realSize*cores))*0.9 << "x)" << std::endl;
    std::cout << "Total Tape Size: ~" << (realSize*chunks)/1000/1000/1000 << " GB" << std::endl;

    // Approx. Time to Finish
    adjoints.resize(a->getRam(), 0);
    long long oneChunk2 = 0;
    #pragma omp parallel for default (none) shared(adjoints, a, oneChunk2)
    for (int i = 0; i < 5; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto adj_tmp = adjoints;
        a->interpret(adj_tmp);
        auto stop = std::chrono::high_resolution_clock::now();
        #pragma omp critical
        oneChunk2 += std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    }
    oneChunk2 = oneChunk2 / 5;

    auto tapeTime = oneChunk;
    auto reversalTime = oneChunk2;
    int ef = cores-1 == 0 ? cores : cores-1;

    std::cout << "CheckpointReversalTime: " << reversalTime << "ms" << std::endl;
    std::cout << "EfficiencyFactor: " << ((double)tapeTime / (ef)) / (reversalTime) << " (Optimal: " << ceil(((double)tapeTime / reversalTime)+1) << " cores)" << std::endl;

    std::cout << "Estimated Execution Time: " << ((double)(oneChunk2)*chunks + totalTime) << "ms" << std::endl;
    delete a;
}

#endif //ADJOINT_PROFILER_HPP
