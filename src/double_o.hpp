#ifndef PROTO_DATA_HPP
#define PROTO_DATA_HPP


#include <math.h>
#include <dag.hpp>

/**
 * Data is a datatype that represents a double and all the operations on a double
 * we redefine + - * etc. to generate a dag
 */
class double_o {
private:
    dag* g = nullptr;
    double value = 0;
    bool isL = false;
public:
    short id = 0;

    double_o() = default;

    double_o(double c): value(c) {
        g = nullptr;
    }

    double_o(double c, dag* g): value(c) {
        g = g;
    }

    double_o(const double_o &d) {
        if (d.g != nullptr) g = d.g;
        value = d.value;
        id = d.id;
        isL = true;
    }

    double getValue() {
        return value;
    }

    void updateBandwidth(const double_o &c) {
        if (isL || c.isL || g == nullptr) {
            return;
        }

        int band = id - c.id;
        if (band > g->bandwidth) g->bandwidth = band;
    }

    void registerInput(dag* g2) {
        isL = true;
        g = g2;
        id = g->getId(isL);

        g->v.push_back(0);
        g->v.push_back(id);
    }

    void record_res(int count) {
        if (g == nullptr) return;
        if (!isL || id == 0) {
            id = g->getId(isL);
        }

        g->v.push_back(count);
        g->v.push_back(id);
    }

    void record_arg(double deriv) const {
        if(g != nullptr) {
            g->v.push_back(id);
            g->d.push_back(deriv);
        }
    }

    friend double_o operator+(const double_o &d1, const double_o &d2) {
        double_o res;
        if (d1.g != nullptr && d2.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(1);
            d2.record_arg(1);

            res.record_res(2);
            res.updateBandwidth(d1);
            res.updateBandwidth(d2);
        } else if (d1.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(1);

            res.record_res(1);
            res.updateBandwidth(d1);
        } else if (d2.g != nullptr) {
            res.g = d2.g;

            d2.record_arg(1);

            res.record_res(1);
            res.updateBandwidth(d2);
        }
        res.value = d1.value + d2.value;
        return res;
    }
    friend double_o operator-(const double_o &d1, const double_o &d2) {
        double_o res;
        if (d1.g != nullptr && d2.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(1);
            d2.record_arg(-1);

            res.record_res(2);
            res.updateBandwidth(d1);
            res.updateBandwidth(d2);
        } else if (d1.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(1);

            res.record_res(1);
            res.updateBandwidth(d1);
        } else if (d2.g != nullptr) {
            res.g = d2.g;

            d2.record_arg(-1);

            res.record_res(1);
            res.updateBandwidth(d2);
        }
        res.value = d1.value - d2.value;
        return res;
    }
    friend double_o operator-(const double_o &d1) {
        double_o res;
        if (d1.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(-1);

            res.record_res(1);
            res.updateBandwidth(d1);
        }
        res.value = -d1.value;
        return res;
    }
    friend double_o operator*(const double_o &d1, const double_o &d2) {
        double_o res;
        if (d1.g != nullptr && d2.g != nullptr) {
            res.g = d1.g;
            if (d1.id == d2.id) {
                d1.record_arg(2 * d1.value);
                res.record_res(1);
            } else {
                d1.record_arg(d2.value);
                d2.record_arg(d1.value);
                res.record_res(2);
            }
        } else if (d1.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(d2.value);

            res.record_res(1);
            res.updateBandwidth(d1);
        } else if (d2.g != nullptr) {
            res.g = d2.g;

            d2.record_arg(d1.value);

            res.record_res(1);
            res.updateBandwidth(d2);
        }
        res.value = d1.value * d2.value;
        return res;
    }

    double_o& operator=(const double_o &d1) {
        if (&d1 == this) return *this;

        if (d1.g != nullptr) g = d1.g;

        value = d1.value;

        isL = true;

        if (d1.g != nullptr) {
            d1.record_arg(1);
            record_res(1);
        }

        return *this;
    }

    friend double_o sin(const double_o &d1) {
        double_o res;
        if (d1.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(cos(d1.value));

            res.record_res(1);
            res.updateBandwidth(d1);
        }

        res.value = sin(d1.value);
        return res;
    }
    friend double_o cos(const double_o &d1) {
        double_o res;
        if (d1.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(-sin(d1.value));

            res.record_res(1);
            res.updateBandwidth(d1);
        }

        res.value = cos(d1.value);
        return res;
    }
    friend double_o operator/(const double_o &d1, const double_o &d2) {
        double_o res;
        if (d1.g != nullptr && d2.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(1/d2.value);
            d2.record_arg(-d1.value/(d2.value*d2.value));
            res.record_res(2);

        } else if (d1.g != nullptr) {
            res.g = d1.g;

            d1.record_arg(1/d2.value);

            res.record_res(1);
            res.updateBandwidth(d1);
        } else if (d2.g != nullptr) {
            res.g = d2.g;

            d2.record_arg(-d1.value/(d2.value*d2.value));

            res.record_res(1);
            res.updateBandwidth(d2);
        }
        res.value = d1.value / d2.value;
        return res;
    }

    friend double_o pow(const double_o &d1, const int exponent) {
        /**
         * The newly generated output
         */
        double_o res;

        /**
         * Test if the provided input is currently being recorded on a dag.
         */
        if (d1.g != nullptr) {
            // the output should also be recorded in that case.
            res.g = d1.g;

            // the derivative in direction of d1
            d1.record_arg(exponent * pow(d1.value, exponent-1));
            res.record_res(1);

        }
        // real operation
        res.value = pow(d1.value, exponent);
        return res;
    }

    // ADDITIONAL LOGIC / No Overloading, but it is needed to function as a double
    friend bool operator<(const double_o &d1, const double &d2) {
        return d1.value < d2;
    }


    friend std::ostream& operator<<(std::ostream& os, const double_o& d) {
        os << d.value;
        return os;
    }
};

#endif //PROTO_DATA_HPP
