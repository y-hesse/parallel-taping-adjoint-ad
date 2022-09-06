#ifndef ADJOINT_CHECKPOINT_HPP
#define ADJOINT_CHECKPOINT_HPP

#include <vector>
#include <dag.hpp>
#include <double_o.hpp>
#include <limits>

/**
 * The Checkpoint class consists of a vector of inputs / outputs
 * These "inputs" do not need to correspond to actual primal inputs but rather to used variables in the code
 * When using Checkpoints and when writing primals make sure to always keep the sive of the input vector consistent throught
 * the runtime of the primal.
 */
class checkpoint {
public:
    /**
     *  @param inputs
     *  The current state of the method saved in a 2D double Vector
     */
    std::vector<double> inputs;
    /**
     * @param from
     * the start of the checkpoint
     */
    uint64_t from;
    /**
     * @param to
     * The optional end of the checkpoint if it is intended to be used for overloading
     * the overloading will only progress to this state
     */
    uint64_t to = 0;

    checkpoint() = default;

    /**
     * Copy Constructor
     * @param c
     */
    checkpoint(const checkpoint& c) {
        inputs = c.inputs;
        from = c.from;
        to = c.to;
    }

    /**
     * Constructor
     * @param in input overloaded double type
     * @param from start of checkpoint
     */
    checkpoint(std::vector<double_o>& in, uint64_t from) {
        inputs = std::vector<double>(in.size());
        for (int i = 0; i < in.size(); i++) {
            inputs[i] = in[i].getValue();
        }
        this->from = from;
    }

    /**
     * Constructor
     * @param in input overloaded double type
     * @param from start of checkpoint
     * @param to the end of the checkpoint (optional)
     */
    checkpoint(std::vector<double_o>& in, uint64_t from, uint64_t to) {
        inputs = std::vector<double>(in.size());
        for (int i = 0; i < in.size(); i++) {
            inputs[i] = in[i].getValue();
        }
        this->from = from;
        this->to = to;
    }

    /**
     * Constructor
     * @param in input default double type
     * @param from start of checkpoint
     */
    checkpoint(std::vector<double>& in, uint64_t from) {
        inputs = in;
        this->from = from;
    }

    /**
     * Constructor
     * @param in input overloaded double type
     * @param from start of checkpoint
     * @param to the end of the checkpoint (optional)
     */
    checkpoint(std::vector<double>& in, uint64_t from, uint64_t to) {
        inputs = in;
        this->from = from;
        this->to = to;
    }

    /**
     * If the Checkpoint is started with a dag this dag will be used to record
     * this checkpoint from the start "to" the end
     * This routine initializes the overloaded doubles and registers them on the dag
     * also fills in the array and from / to params
     * @param g to be Overloaded DAG pointer
     * @param c will provide the current state of the primal
     * @param from will provide the start
     * @param to will provide the to / end
     */
    void start(dag *g, std::vector<double_o> &c, uint64_t &from, uint64_t &to) {
        from = this->from;
        to = this->to;

        c = std::vector<double_o> (inputs.size());
        for (int i = 0; i < inputs.size(); i++) {
            c[i] = inputs[i];
            c[i].registerInput(g);
        }
    }

    /**
     * Starting the Checkpoint without the DAG
     * @param c will provide the current state of the primal (as overloaded double)
     * @param from will provide the start
     * @param to will provide the to / end
     */
    void start(std::vector<double_o> &c, uint64_t &from, uint64_t &to) {
        from = this->from;
        to = this->to;

        c = std::vector<double_o> (inputs.size());
        for (int i = 0; i < inputs.size(); i++) {
            c[i] = inputs[i];
        }
    }

    /**
     * Starting the Checkpoint without the DAG
     * @param c will provide the current state of the primal (as double)
     * @param from will provide the start
     * @param to will provide the to / end
     */
    void start(std::vector<double> &c, uint64_t &from, uint64_t &to) {
        from = this->from;
        to = this->to;

        c = std::vector<double> (inputs.size());
        for (int i = 0; i < inputs.size(); i++) {
            c[i] = inputs[i];
        }
    }

    /**
     * Human readable non binary output of the checkpoint
     * Potential change in the future
     */
    friend std::ostream& operator<<(std::ostream& os, const checkpoint& check) {
        os << "{";

        os << check.from;
        os << ";";
        os << check.to;
        os << ";";

        os.precision(std::numeric_limits<double>::max_digits10 - 1);

        for (auto it = check.inputs.begin(); it != check.inputs.end(); it++) {
            os << std::scientific << *it;
            os << ",";
        }

        os << "}";
        return os;
    }

    /**
     * Human readable non binary input of the checkpoint
     * Potential change in the future
     */
    friend std::istream& operator>>(std::istream& is, checkpoint& check) {
        char c;
        double f;

        is >> std::ws;

        check.inputs = std::vector<double>(0);


        if (is.get() == '{') {
            is >> check.from;
            is.get();
            is >> check.to;
            is.get();

            while(is.peek() != '}') {
                is >> f;
                is.get(c);
                check.inputs.push_back(f);
            }
        } else {
            std::cout << c << "error";
        }
        return is;
    }

    /**
     * For debugging and comparisons
     * @param d1
     * @param d2
     * @return
     */
    friend bool operator==(const checkpoint &d1, const checkpoint &d2) {
        if (d1.from == d2.from && d1.to == d2.to && d1.inputs.size() == d2.inputs.size()) {
            if (d1.inputs == d2.inputs) {
                return true;
            }
        }
        return false;
    }
};

#endif //ADJOINT_CHECKPOINT_HPP

