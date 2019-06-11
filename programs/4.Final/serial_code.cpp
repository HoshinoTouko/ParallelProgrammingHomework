#include<iostream>

double f(double a) {
    return a * a;
}

int main() {
    int a, b, h, n;
    float result;
    float temp_x;

    h = (b - a) / n;
    result = (f(a) + f(b)) / 2.0;
    for (int i = 0; i < n; i++) {
        temp_x = a + i * h;
        result += f(temp_x);
    }
    result *= h;

    return 0;
}