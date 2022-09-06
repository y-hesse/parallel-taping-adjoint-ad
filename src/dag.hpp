#ifndef PROTO_DAG_HPP
#define PROTO_DAG_HPP

#include <cstdint>
#include <vector>
#include <iostream>

typedef short id;

#ifndef THRESHOLD
#define THRESHOLD 10000
#endif

/**
 * This Data-Structure is the main DCG that is then interpreted as a DAG.
 * Uses a custom Datatype to overload operations on double's allowing the code to be overloaded.
 */
class dag {
private:
    id counter = 0;
    id persistent_adjoints = -1;
public:
    /**
     * Contains a list of derivatives in order of overloading
     */
    std::vector<double> d;
    /**
     * Contains the dag interpreted in reverse containing the identifier of the node then the
     * amount of nodes pointing at it, followed by a list of said nodes.
     */
    std::vector<id> v;
    /**
     * The current bandwidth (vector size) needed for adjoint reversal
     */
    int bandwidth = 1;

    dag() = default;

    /**
     * Transform real id -> adjoint vector position
     * @param id the overloaded double id
     * @return the real position it has on the adjoint vector
     */
    int adjoint_id(id id) const {
        if (id < 0) return -id-1;
        return id % bandwidth - persistent_adjoints - 1;
    }

    /**
     * Required size of the
     * @return
     */
    int getRam() const {
        return bandwidth - persistent_adjoints - 1;
    }

    /**
     * Used by overloaded double type for the next id
     * @param c if the double_o is a left side (true / persistent) or right side (false / non persistent)
     * @return the id given to this new overloaded double
     */
    id getId(bool c) {
        if (c) {
            return persistent_adjoints--;
        } else {
            /*
             * The number 10000 is chosen arbitrarily by the programmer (Yannik Hesse),
             * although there are primals which this might cause conflicts with, the general idea is
             * that no singular expression should contain more than 10000 elemental operations (lol)
             * The reason this is done, is to support extremely large tapes with ease and not be limited to the size of
             * a short (id). Solutions choosing an int or int64_t as a counter type are found not to be sufficiently scalable and
             * cause significant additional load on the memory. If there is any real need to extend the persistent memory, as in there
             * is a primal that uses more than 65.000 distinct variables changing the id datatype from short to int is an easy step.
             */
            if (counter >= THRESHOLD + bandwidth && counter % bandwidth == 0) {
                counter = 0;
            }
            return counter++;
        }
    }

    /**
     * Interpret the DCG by reversing all Nodes and Edges. An Adjoint vector is needed with at least the size of
     * the bandwidth to successfully interpret the DAG
     * @param adj Vector of Adjoints at least the size of the current bandwidth
     */
    void interpret(std::vector<double> &adj) {
        auto it = v.rbegin();
        auto it2 = d.rbegin();

        while(it != v.rend()) {
            int idx = adjoint_id(*it++);
            int c = *it++;
            double ak = adj[idx];
            if (c != 0) adj[idx] = 0;
            for (int i = 0; i < c; ++i) {
                int id2 = adjoint_id(*it++);
                double t = *it2++;
                adj[id2] += ak*t;
            }
        }
    }

    /**
     * Used to get the memory usage of the DCG, as this is a large limiting factor
     * @return memory usage of DCG
     */
    uint64_t getMemorySize() {
        return sizeof(this)+sizeof(double)*(d.size())+sizeof(id)*v.size()+sizeof(double)*getRam();
    }




    // Output as vector
    /*
    friend std::ostream& operator<<(std::ostream& os, const dag& d) {
        os << "v (";
        for(auto c : d.v) {
            os << c << ", ";
        }
        os << ")" << std::endl;

        os << "d (";
        for(auto c : d.d) {
            os << c << ", ";
        }
        os << ")" << std::endl;

        return os;
    }
    /*
    /*
     * Output the as a DCG in the .dot format
     * this is useful for low level debugging
     */
    friend std::ostream& operator<<(std::ostream& os, const dag& dd) {
        os << "digraph G {" << std::endl << "rankdir=LR;" << std::endl;

        auto it = dd.v.rbegin();
        auto it2 = dd.d.rbegin();
        while(it != dd.v.rend()) {
            int id = *it++;
            int c = *it++;
            for (int i = 0; i < c; ++i) {
                os << "\"" << *it++ << "\"->\"" << id << "\" [label=\"" << *it2++ <<  "\"];" << std::endl;
            }
        }

        os << "}" << std::endl;

        return os;
    }
};


#endif //PROTO_DAG_HPP
