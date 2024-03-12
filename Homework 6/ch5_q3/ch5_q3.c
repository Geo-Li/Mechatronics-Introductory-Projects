#include <xc.h>

int main(void) {
    // Define variables for different types
    char c1=5, c2=6, c3;
    int i1=5, i2=6, i3;
    long long int j1=5, j2=6, j3;
    float f1=1.01, f2=2.02, f3;
    long double d1=1.01, d2=2.02, d3;

    // Perform add, subtract, multiply, and divide for each of the data type
    c3 = c1+c2;
    c3 = c1-c2;
    c3 = c1*c2;
    c3 = c1/c2;

    i3 = i1+i2;
    i3 = i1-i2;
    i3 = i1*i2;
    i3 = i1/i2;

    j3 = j1+j2;
    j3 = j1-j2;
    j3 = j1*j2;
    j3 = j1/j2;

    f3 = f1+f2;
    f3 = f1-f2;
    f3 = f1*f2;
    f3 = f1/f2;

    d3 = d1+d2;
    d3 = d1-d2;
    d3 = d1*d2;
    d3 = d1/d2;

    return 0;
}

