#ifndef _UTILS_INCLUDED_
#define _UTILS_INCLUDED_

const double pi=3.141592653589793;

inline double norm(vector<double_o>& v) {
    int n=v.size();
    double r=0;
    for (int i=1;i<n-1;i++) r= r + v[i].getValue()*v[i].getValue();
    return r;
}

inline double norm(vector<double>& v) {
    int n=v.size();
    double r=0;
    for (int i=1;i<n-1;i++) r= r + v[i]*v[i];
    return r;
}

#endif