#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>


double pow(double x, int y) {

    double result = 1.0;
    if(y == 0)
        return 1.0;
    int i;
    for (i = 0; i < y; i++) {
        result *= x;
    }
    return result;
}

double log(double x) {

    if(x < 0.0)
        return -1.0;

    double r;
    double ans =0.0;

    int i, n=21;

    for (i = 1; i < n; i++) {
      r = pow(x-1, i) / i;
      if(i%2 == 0){
        ans = ans - r;
      }
      else {
        ans = ans + r;
      }
    }

    return ans;
}

double expdev(double lambda) {

    double dummy;
    do
        dummy= (double) rand() / 32767;
    while (dummy == 0.0);
    return -log(dummy) / lambda;
}