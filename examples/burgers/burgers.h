#pragma once

#include <vector>
#include <cmath>
#include <functional>
using namespace std;

#include "utils.h"
#include "gauss.h"

namespace burgersFunction {

    template<typename T>
    void primal(std::vector<T> &inout);

    void primal(checkpoint c, dag* D);
    void primal(checkpoint c, std::function<void(checkpoint)> addCheckpoint);
}


//*** right-hand side of ODE dy/dt = v d^2 y / d x^2 - y dy / dx
//***                                `---,---------´   `---,---´
//***                                    diffusion         advection
template <typename T>
inline void g(const double& d, const vector<T>& y, vector<T>& r, T& diffusion, T& advection) {
  int n=y.size();
  for (int i=1;i<n-1;i++) {
    //*** central finite difference scheme for diffusion term on
    //*** equidistant 1D grid
    diffusion = d*(n-1)*(n-1)*(y[i-1]-2*y[i]+y[i+1]);
    //*** first-order upwind scheme for advection term on equidistant
    //*** 1D grid
    advection = -y[i]*(n-1);
    if (advection < 0) {  advection = advection *(y[i]-y[i-1]); }
    else               { advection = advection * (y[i+1]-y[i]); }
    r[i] = diffusion + advection;
  }
}

// tangent of g (hand-written)
template <typename T>
inline void g_t(const double& d, const vector<T>& y, const vector<T>& y_t, vector<T>& r_t, T& diffusion_t, T& advection, T& advection_t) {
  int n=y.size();
  for (int i=1;i<n-1;i++)  {
    diffusion_t = d*(n-1)*(n-1)*(y_t[i-1]-2*y_t[i]+y_t[i+1]);
    advection = -y[i]*(n-1);
    advection_t = -y_t[i]*(n-1);
    if (advection < 0) { advection_t = advection * (y_t[i]-y_t[i-1]) + advection_t * (y[i]-y[i-1]); }
    else               { advection_t = advection * (y_t[i+1]-y_t[i]) + advection_t * (y[i+1]-y[i]); }
    r_t[i] = diffusion_t + advection_t;
  }
}

//*** Jacobian of r with respect to y (i.e. Jacobian of implementation g)
//***  - for the computation, the tangent of g (i.e. g_t) is used
//***  - A is known to be a tridiagonal matrix and uses custom data format, it stores row-wise
//***       a b
//***       c d e
//***         f g h
//***           i j k
//***             l m
//***    where in our case (a, b, c, k, l, m) = 0
//***  - Curtis/Powell/Reid(CPR)-algorithm is used to compute compressed Jacobian
//***  - if transpose=true, the transposed Jacobian is returned in A using the same data format.
template <typename T>
inline void dgdy(const double& d, const vector<T>& y, vector<T>& A, vector<T>& y_t, vector<T>& r_t, T& diffusion_t, T& advection, T& advection_t,
                 bool transpose=false) {
  int n=y.size();
  for (int i=0;i<3;i++) {
      for (int j = 0; j < n; ++j) {
          y_t[j] = 0;
      }
    //*** CPR seeding
    for (int j=i+1;j<n;j+=3) y_t[j]=1;
    
    //*** computes compressed column in r_t
    g_t(d,y,y_t,r_t, diffusion_t, advection, advection_t);

    for (int j=i+1;j<n-1;j+=3) {
      A[j*3]=r_t[j]; // diagonal elements (e.g. g)
      if (transpose) {
        //*** store column r_t in row
        A[j*3-1]=r_t[j-1]; // left of diagonal (i.e. f)
        A[j*3+1]=r_t[j+1]; // right of diagonal (i.e. h)
      } else {
        A[(j-1)*3+1]=r_t[j-1]; // above diagonal (i.e. e)
        A[(j+1)*3-1]=r_t[j+1]; // below diagonal (i.e. i)
      }
    }
  }
}

//*** residual of nonlinear system
//*** - y_prev is solution of previous timestep
template <typename T>
inline void f(const int m, const double& d,
    const vector<T>& y, const vector<T>& y_prev, vector<T>& r, T& diffusion, T& advection) {
  int n=y.size();
  g(d,y,r, diffusion, advection);
  r[0]=y[0];
  for (int i=1;i<n-1;i++) r[i]=y[i]-y_prev[i]-r[i]/m;
  r[n-1]=y[n-1];
}

//*** Jacobian of residual of nls wrt. state (i.e. y)
//*** - see description of dgdy
template <typename T>
inline void dfdy(const int m, const double& d,
                 const vector<T>& y,
                 vector<T>& A, vector<T>& y_t, vector<T>& r_t, T& diffusion_t, T& advection, T& advection_t, bool transpose=false) {
  int n=y.size();
  dgdy(d,y,A,y_t, r_t, diffusion_t, advection, advection_t, transpose);
  for (auto& e : A) e= e/-m;
  for (int i=1;i<n-1;i++) A[i*3]=A[i*3]+1;
}

//*** Newton solver for nls
template <typename T>
inline void newton(const int m, const double& d, const vector<T>& y_prev, vector<T>& y, vector<T>& A, vector<T>& r, vector<T>& y_t, vector<T>& r_t, T& diffusion_t, T& advection, T& advection_t) {
    int n=y.size();
    const double eps=1e-14*1e-14;
    f(m,d,y,y_prev,r, diffusion_t, advection);
    while (norm(r)>eps) {
        dfdy(m,d,y,A, y_t, r_t, diffusion_t, advection, advection_t);
        LU(A); FS(A,r); BS(A,r);
        for (int i=1;i<n-1;i++) y[i]=y[i]-r[i];
        f(m,d,y,y_prev,r, diffusion_t, advection);
    }
}

//*** implicit Euler integration
template <typename T>
inline void burgers(const int m, const double& d, vector<T>& y, bool output_progress=true) {
    int n=y.size();
    vector<T> A((n-2)*3+4,0), r(n,0), r_t(n), y_t(n), y_prev(n);
    T diffusion_t;
    T advection;
    T advection_t;
    for (int j = 0; j < m; j++) {
        for (int i = 0; i < n; ++i) {
            y_prev[i] = y[i];
            r[i] = 0;
        }
        for (int i = 0; i < (n-2)*3+4; ++i) {
            A[i] = 0;
        }
        newton(m, d, y_prev, y, A, r, y_t, r_t, diffusion_t, advection, advection_t);
    }
}

