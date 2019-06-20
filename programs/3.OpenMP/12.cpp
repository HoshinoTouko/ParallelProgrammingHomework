#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <ctime>
#include <omp.h>

void Init(double A[], double b[], double x[], int n) {
    int i, j;

    for (i = 0; i < n; i++)
        x[i] = 1.0;

    srand(1);
    memset(A, 0, n*n*sizeof(double));
    for (i = 0; i < n; i++) {
        A[i*n+i] = n/10.0;
        for (j = i+1; j < n; j++)
            A[i*n + j] = rand()/((double) RAND_MAX);
    }

    for (i = 0; i < n; i++) {
        b[i] = 0;
        for (j = i; j < n; j++)
            b[i] += A[i*n + j]*x[j];
    }

    memset(x, 0, n*sizeof(double));
}

void Solve_row(double A[], double b[], double x[], int n, int thread_count) {
    int i, j;
    double tmp;

    #  pragma omp parallel num_threads(thread_count) \
    default(none) private(i, j) shared(A, b, n, x, tmp)
    for (i = n-1; i >= 0; i--) {
        #     pragma omp single
        tmp = b[i];
        #     pragma omp for reduction(+: tmp) schedule(runtime)
        for (j = i+1; j < n; j++)
            tmp += -A[i*n+j]*x[j];
        #     pragma omp single
        {
            x[i] = tmp/A[i*n+i];
#        ifdef DEBUG
            printf("x[%d] = %.1f\n", i, x[i]);
#        endif
        }
    }
}

void Solve_col(double A[], double b[], double x[], int n, int thread_count) {
    int i, j;

    #  pragma omp parallel num_threads(thread_count) \
    default(none) private(i,j) shared(A, b, x, n)
    {
        #     pragma omp for
        for (i = 0; i < n; i++)
            x[i] = b[i];

        for (j = n-1; j >= 0; j--) {
            #        pragma omp single
            x[j] /= A[j*n+j];
            #        pragma omp for schedule(runtime)
            for (i = 0; i < j; i++)
                x[i] += -A[i*n + j]*x[j];
        }
    }
}

void Print_mat(char title[], double A[], int n) {
    int i, j;

    printf("%s:\n", title);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
            printf("%4.1f ", A[i*n+j]);
        printf("\n");
    }
    std::cout<<std::endl;
}

void Print_vect(char title[], double x[], int n) {
    int i;

    printf("%s ", title);
    for (i = 0; i < n; i++)
        printf("%.1f ", x[i]);
    std::cout<<std::endl;
}

int main(int argc, char* argv[]) {
    int n = 1000, thread_count = 4;
    double *A, *b, *x;
    double start, end;

    A = (double *)malloc(n*n*sizeof(double));
    b = (double *)malloc(n*sizeof(double));
    x = (double *)malloc(n*sizeof(double));

    Init(A, b, x, n);

    memset(x, 0, n*sizeof(double));
    start = clock();
    Solve_row(A, b, x, n, thread_count);
    end = clock();
    std::cout<<"Row: "<<end-start<<std::endl;

    memset(x, 0, n*sizeof(double));
    start = clock();
    Solve_col(A, b, x, n, thread_count);
    end = clock();
    std::cout<<"Col: "<<end-start<<std::endl;

    free(A);
    free(b);
    free(x);

    return 0;
}
