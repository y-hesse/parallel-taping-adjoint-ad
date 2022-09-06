#ifndef ADJOINT_AAD_HPP
#define ADJOINT_AAD_HPP

#include <vector>
#include <iostream>
#include <string>
#include <chrono>
#include <primal/primal.hpp>
#include <checkpoint.hpp>
#include <verify.hpp>
#include <naive/checkpointLoader.hpp>
#include <disk/checkpointLoader.hpp>
#include <memory/checkpointLoader.hpp>
#include <hybrid/checkpointLoader.hpp>
#include <thread>
#include "omp.h"

/**
 * PRIMAL the namespace of the primal function
 */
using namespace PRIMAL;

/**
 * The Checkpoint generation method to be used
 */
using namespace CHECKPOINTING;

/**
 *  Debug Level := Minimal | Verbose | Timing
 */
const string debug = DEBUG;

/**
 * Adjoint Algorithmic Differentiation routine
 * @param in Input vector
 * @param adjoints Adjoint vector, outputs will be placed here
 * @param totalTime Elapsed aad time in milliseconds, timed using the chrono library
 */
void aad(std::vector<double> in, std::vector<double>& adjoints, uint64_t& totalTime) {

    /*
     * Copy the Adjoints Vector for Tests later
     */
    std::vector<double> yAd = adjoints;

    auto start = std::chrono::high_resolution_clock::now();
    auto reversal = std::chrono::high_resolution_clock::now();
    auto reversal2 = reversal;
    if (debug != "Minimal") {
        std::cout << "Using: " << cores << " core(s) with a size of " << size << std::endl;
        std::cout << "Using Mode: " << mode << std::endl;
    }

    auto startLoading = std::chrono::high_resolution_clock::now();

    /*
     * Create a checkpointLoader, and provide the CPU core amount to be used when generating
     */
    checkpointLoader c(cores);

    /*
     * Start the checkpointLoader with the seeded input vector
     */
    c.recordLoader(in);

    auto stopLoading = std::chrono::high_resolution_clock::now();
    auto startOverloading = std::chrono::high_resolution_clock::now();
    auto startReversal = std::chrono::high_resolution_clock::now();
    auto startChunkOne = std::chrono::high_resolution_clock::now();
    auto stopChunkOne = std::chrono::high_resolution_clock::now();
    auto startIdle = std::chrono::high_resolution_clock::now();
    auto stopIdle = std::chrono::high_resolution_clock::now();
    double idleMs = 0;

    /*
     * Parallel main aad part
     * Reverse all Chunks, Overload and then Reverse the generated DAG
     * Utilizing OpenMP Multithreading
     */
#pragma omp parallel for ordered schedule(dynamic, 1) num_threads(cores) default(none) shared(debug, c, size, windowThreadSize, std::cout, adjoints, chunks, startReversal, stopChunkOne, startIdle, stopIdle, idleMs, yAd)
    for (int64_t i = (int64_t)chunks; i >= 1; --i) {
        if (debug == "Verbose") {
            printf("%d(%d) ", i, omp_get_thread_num());
        }
        checkpoint check;
        /*
         * Checkpoint for chunk i
         */
        c.getCheckpoint(i, check);

        /*
         * Overloading run
         */
        dag* g = new dag();
        primal(check, g);
#pragma omp ordered
        {
            stopIdle = std::chrono::high_resolution_clock::now();
            if (i == (int64_t)chunks) {
                adjoints.resize(g->getRam(), 0);
                yAd.resize(g->getRam(), 0);
                stopChunkOne = std::chrono::high_resolution_clock::now();
                startReversal = std::chrono::high_resolution_clock::now();
            } else {
                idleMs += std::chrono::duration_cast<std::chrono::milliseconds>(stopIdle - startIdle).count();
            }
            /*
             * Ordered reversal run
             */
            g->interpret(adjoints);
            startIdle = std::chrono::high_resolution_clock::now();
        };

        delete g;
    }
    std::cout << std::endl << std::endl;

    auto stop = std::chrono::high_resolution_clock::now();

    if (debug == "Timing" || debug == "Verbose") {
        std::cout << "Checkpoint Generation Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stopLoading - startLoading).count() << std::endl;
        std::cout << "Chunk One Overloading Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stopChunkOne - startChunkOne).count() << std::endl;
        std::cout << "Total Reversal Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - startReversal).count() << std::endl;
        std::cout << "Reversal Idle Time: " << idleMs << std::endl;
        std::cout << "Total Execution Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
    }
    totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    if (debug == "Verbose") {
        std::cout << std::endl << std::endl << std::endl << "Adjoints:" << std::endl;

        /*
         * Optionally output the Adjoint
         */
        for(auto d : adjoints) {
            std::cout << d << " ";
        }
    }

    /*
     * Optionally run a test if the provided Results are correct
     */
    if (debug == "Timing") {
        std::cout << std::endl << "Testing Adjoints:";
        std::cout << std::endl << verifyPercent(in, yAd, adjoints) << " Correct" << std::endl;
    }

    adjoints.resize(yAd.size());
}

/**
 * Adjoint Algorithmic Differentiation routine
 * @param in Input vector
 * @param adjoints Adjoint vector, outputs will be placed here
 */
void aad(std::vector<double> in, std::vector<double>& adjoints) {
    uint64_t t = 0;
    aad(in, adjoints, t);
}

/**
 * Adjoint Algorithmic Differentiation routine
 * @param in Input vector
 * @param adjoints Vector of Adjoint vector, outputs will be placed there, useful if you have multiple outputs.
 * @param totalTime Elapsed aad time in milliseconds, timed using the chrono library
 */
void aadMulti(std::vector<double> in, std::vector<std::vector<double>>& adjoints, uint64_t& totalTime) {

    auto yAd = adjoints;

    auto start = std::chrono::high_resolution_clock::now();
    auto reversal = std::chrono::high_resolution_clock::now();
    auto reversal2 = reversal;
    if (debug != "Minimal") {
        std::cout << "Using: " << cores << " core(s) with a size of " << size << std::endl;
        std::cout << "Using Mode: " << mode << std::endl;
    }

    auto startLoading = std::chrono::high_resolution_clock::now();

    /*
     * Create a checkpointLoader, and provide the CPU core amount to be used when generating
     */
    checkpointLoader c(cores);

    /*
     * Start the checkpointLoader with the seeded input vector
     */
    c.recordLoader(in);

    auto stopLoading = std::chrono::high_resolution_clock::now();
    auto startOverloading = std::chrono::high_resolution_clock::now();
    auto startReversal = std::chrono::high_resolution_clock::now();
    auto startChunkOne = std::chrono::high_resolution_clock::now();
    auto stopChunkOne = std::chrono::high_resolution_clock::now();
    auto startIdle = std::chrono::high_resolution_clock::now();
    auto stopIdle = std::chrono::high_resolution_clock::now();
    double idleMs = 0;

    /*
     * Parallel main aad part
     * Reverse all Chunks, Overload and then Reverse the generated DAG
     * Utilizing OpenMP Multithreading
     */
    omp_set_nested(true);
#pragma omp parallel for ordered schedule(dynamic, 1) num_threads(cores) default(none) shared(debug, c, size, windowThreadSize, std::cout, adjoints, chunks, startReversal, stopChunkOne, startIdle, stopIdle, idleMs, yAd)
    for (int64_t i = (int64_t)chunks; i >= 1; --i) {
        if (debug == "Verbose") {
            printf("%d(%d) ", i, omp_get_thread_num());
        }
        checkpoint check;
        /*
         * Checkpoint for chunk i
         */
        c.getCheckpoint(i, check);

        /*
         * Overloading run
         */
        dag* g = new dag();
        primal(check, g);

#pragma omp ordered
        {
            stopIdle = std::chrono::high_resolution_clock::now();
            if (i == (int64_t)chunks) {
                for (int j = 0; j < adjoints.size(); ++j) {
                    adjoints[j].resize(g->getRam(), 0);
                    yAd[j].resize(g->getRam(), 0);
                }
                stopChunkOne = std::chrono::high_resolution_clock::now();
                startReversal = std::chrono::high_resolution_clock::now();
            } else {
                idleMs += std::chrono::duration_cast<std::chrono::milliseconds>(stopIdle - startIdle).count();
            }
            /*
             * Ordered reversal run
             */
            #pragma omp parallel for default (none) shared(adjoints, g)
            for (int j = 0; j < adjoints.size(); ++j) {
                g->interpret(adjoints[j]);
            }
            startIdle = std::chrono::high_resolution_clock::now();
        };

        delete g;
    }
    std::cout << std::endl << std::endl;

    auto stop = std::chrono::high_resolution_clock::now();

    if (debug == "Timing" || debug == "Verbose") {
        std::cout << "Checkpoint Generation Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stopLoading - startLoading).count() << std::endl;
        std::cout << "Chunk One Overloading Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stopChunkOne - startChunkOne).count() << std::endl;
        std::cout << "Total Reversal Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - startReversal).count() << std::endl;
        std::cout << "Reversal Idle Time: " << idleMs << std::endl;
        std::cout << "Total Execution Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
    }
    totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    if (debug == "Verbose") {
        std::cout << std::endl << std::endl << std::endl << "Adjoints:" << std::endl;

        /*
         * Optionally output the Adjoint
         */
        for (auto d : adjoints) {
            std::cout << "Adjoints: " << std::endl;
            for (auto e : d) {
                std::cout << e << " ";
            }
            std::cout << std::endl;
        }
    }

    /*
     * Optionally run a test if the provided Results are correct
     */
    if (debug == "Timing") {
        std::cout << std::endl << "Testing Adjoints:" << std::endl;
        for (int i = 0; i < adjoints.size(); ++i) {
            std::cout << i << ": " << verifyPercent(in, yAd[i], adjoints[i]) << " Correct" << std::endl;
        }
    }
}

#endif //ADJOINT_AAD_HPP
