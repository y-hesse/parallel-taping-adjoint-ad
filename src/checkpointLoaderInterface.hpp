#ifndef ADJOINT_CHECKPOINTLOADERINTERFACE_HPP
#define ADJOINT_CHECKPOINTLOADERINTERFACE_HPP

#include <vector>
#include <checkpoint.hpp>

/**
 * An interface or blue print for checkpointLoader's
 * A checkpointLoader is a class that's purpose is to
 * create checkpoints and store / retrieve them.
 * The reason for this Interface existing is that there are multiple ways
 * to offload this checkpoint recording and to make it as smooth as
 * possible.
 * That is why an interface is used. To make it easy for the user to switch around
 * and expand this area of the software.
 */
class checkpointLoaderInterface {
public:
    /**
     * Called at the start of the derivative, for initialization or complete recording
     */
    virtual void recordLoader(std::vector<double> &input) = 0;

    /**
     * Each thread will request a chunk by providing the chunk id.
     * WARNING! Needs to be thread safe, as it can be called concurrently by different
     * threads
     */
    virtual bool getCheckpoint(uint64_t i, checkpoint &c) = 0;
};

#endif //ADJOINT_CHECKPOINTLOADERINTERFACE_HPP
