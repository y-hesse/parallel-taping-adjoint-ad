#ifndef ADJOINT_VERIFY_HPP
#define ADJOINT_VERIFY_HPP


#include <vector>
#include <cmath>
#include <iostream>
#include "primal/primal.hpp"
#include <cstdlib>
#include <ctime>

bool verifyResults(const std::vector<double> &x, const std::vector<double> &yAd, const std::vector<double> &xAd) {
    std::vector<double> xTan(x.size());
    double deltaH = 0;
    for (int i = 0; i < x.size(); i++) {
        xTan[i] = ((double)(std::rand() % 10000) - 5000) / 1000.;
        deltaH += x[i];
    }

    std::vector<double> delta(x.size());
    for (int i = 0; i < x.size(); i++) {
        delta[i] = pow(2, -15)*deltaH;
    }

    std::vector<double> t1(x.size());
    for (int i = 0; i < x.size(); i++) {
        t1[i] = x[i] + delta[i]*xTan[i];
    }

    std::vector<double> t2 = x;
    PRIMAL::primal(t1);
    PRIMAL::primal(t2);

    std::vector<double> yTan(x.size());
    for (int i = 0; i < x.size(); i++) {
        if (delta[i] != 0) {
            yTan[i] = (t1[i] - t2[i]) / delta[i];
        } else {
            yTan[i] = 0;
        }
    }

    double xT = 0;
    double yT = 0;
    for (int i = 0; i < x.size(); ++i) {
        xT += xTan[i] * xAd[i];
        yT += yTan[i] * yAd[i];
    }

    return (abs(xT - yT) < 0.1);
}

/**
 * A basic Unit-Test to determine if the provided results match the current primal function
 * We calculate the TAD (Tanget Algorithmic Diffrentiation) using a finite difference approximation.
 * We then use these randomly generated tangents to verify the given Adjoints that where provided.
 * @param x original input
 * @param yAd the provided adjoints
 * @param xAd the results of these adjoint calculations
 * @return The total number of successful tries as a percentage over the total tries
 */
double verifyPercent(const std::vector<double> &x, const std::vector<double> &yAd, const std::vector<double> &xAd) {
    std::srand(std::time(nullptr));
    int right = 0;
#pragma omp parallel for default(none) shared(x, yAd, xAd,right)
    for (int i = 0; i < 100; ++i) {
        if (verifyResults(x, yAd, xAd)) {
#pragma omp atomic
            right++;
        }
    }
    return (double)right/100.;
}

void getTan(const std::vector<double> &x, const std::vector<double> &xTan) {
    double deltaH = 0;
    for (int i = 0; i < x.size(); i++) {
        deltaH += x[i];
    }

    std::vector<double> delta(x.size());
    for (int i = 0; i < x.size(); i++) {
        delta[i] = pow(2, -15)*deltaH;
    }

    std::vector<double> t1(x.size());
    for (int i = 0; i < x.size(); i++) {
        t1[i] = x[i] + delta[i]*xTan[i];
    }

    std::vector<double> t2 = x;
    PRIMAL::primal(t1);
    PRIMAL::primal(t2);

    std::vector<double> yTan(x.size());
    for (int i = 0; i < x.size(); i++) {
        if (delta[i] != 0) {
            yTan[i] = (t1[i] - t2[i]) / delta[i];
        } else {
            yTan[i] = 0;
        }
        printf("%f ", yTan[i]);
    }
}

#endif //ADJOINT_VERIFY_HPP
