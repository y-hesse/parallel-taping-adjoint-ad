#include <cstdlib>
#include <iostream>
#include <vector>
#include <cfloat>
#include <cassert>
#include <cmath>
using namespace std;

#include "burgers.h"

int main(int argc, char* argv[]){
  assert(argc==3);
  int n=atoi(argv[1]), m=atoi(argv[2]);
  vector<double> y(n,0);
  for (int i=1;i<n-1;i++) y[i]=sin(2*pi*i/n); 
  const double d=1e-2;
  for(int j=1;j<n-1;j++) {
    //progress::update(static_cast<float>(j)/static_cast<float>(n-1));
    vector<double> yph(y), ymh(y);
    double h=(y[j]<10) ? sqrt(DBL_EPSILON) : sqrt(DBL_EPSILON)*abs(y[j]); 
    ymh[j]-=h; burgers(m,d,ymh, /*output_progress=*/false);
    yph[j]+=h; burgers(m,d,yph, /*output_progress=*/false);
    cout << double(j)/n << " " << (yph[(n-1)/2]-ymh[(n-1)/2])/(2*h) << endl;
  }
  //progress::finish();
  return 0;
}
