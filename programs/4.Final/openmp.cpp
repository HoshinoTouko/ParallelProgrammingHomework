#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <omp.h>

void Trap(double a, double b, int n, double* global_result_p);
double f(double x);

int thread_count = 8;
double dx;

int main(int argc, char *argv[]) {
    double global_result = 0.0;
    int start, end;
    double a, b;
    int n;

    std::cout<<"Input a, b, n\n"<<std::endl;
    std::cin>>a>>b>>n;

    std::cout<<"Standard "<<(b * b * b - a * a * a) / 3<<std::endl;

    dx = (b - a) / n;

    start = clock();
    # pragma omp parallel num_threads(thread_count)
    Trap(a, b, n, &global_result);
    end = clock();

    std::cout<<std::endl;
    std::cout<<"Thread count: "<<thread_count<<std::endl;
    std::cout<<"CPU Time: "<<end - start<<". Result: "<<global_result<<std::endl;

    return 0;
}

void Trap(double a, double b, int n, double* global_result_p) {
    double h, x, my_result;
    double local_a, local_b, local_n;

    int my_rank = omp_get_thread_num();
    int thread_count = omp_get_num_threads();

    local_n = n / thread_count;
    local_a = a + my_rank * local_n * dx;
    local_b = local_a + local_n * dx;
    my_result = (f(local_a) + f(local_b)) / 2.0;
    for (int i = 1; i < local_n; i++) {
        x = local_a + i * dx;
        my_result += f(x);
    }
    my_result *= dx;
    # pragma omp critical
    *global_result_p += my_result;
}

double f(double x) {
    double return_val;

    return_val = x * x;
    return return_val;
}
