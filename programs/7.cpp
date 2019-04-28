#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

const int thread_count = 4;
const int matrix_column = 30000;
const int matrix_row = 30000;

int     m, n;
double* A;
double* x;
double* y;


void Usage(char* prog_name);
void Read_matrix(char* prompt, double A[], int m, int n);
void Read_vector(char* prompt, double x[], int n);
void Gen_matrix(double A[], int m, int n);
void Gen_vector(double x[], int n);
void Print_matrix(char* title, double A[], int m, int n);
void Print_vector(char* title, double y[], double m);


void *Pth_mat_vect_shared(void* rank);
void *Pth_mat_vect_local(void* rank);


int main(int argc, char* argv[]) {
    long       thread;
    pthread_t* thread_handles;
    double     time_start, time_end;

    thread_handles = (pthread_t*)malloc(thread_count*sizeof(pthread_t));

    m = matrix_column;
    n = matrix_row;

    A = (double*)malloc(m * n * sizeof(double));
    x = (double*)malloc(n * sizeof(double));
    y = (double*)malloc((m + 8 * thread_count) * sizeof(double));

    std::cout<<"Generating matrix..."<<std::endl;
    Gen_matrix(A, m, n);
    Gen_vector(x, n);

    // Shared variable
    std::cout<<"Testing matrix multiplication using shared variables."<<std::endl;

    time_start = clock();
    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL,
                       Pth_mat_vect_shared, (void*) thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);
    time_end = clock();

    std::cout<<"Shared variable cost time: "<<time_end-time_start<<std::endl;

    // Local variable
    std::cout<<"Testing matrix multiplication using local variables."<<std::endl;

    time_start = clock();
    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL,
                       Pth_mat_vect_local, (void*) thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);
    time_end = clock();

    std::cout<<"Local variable cost time: "<<time_end-time_start<<std::endl;

    free(A);
    free(x);
    free(y);

    return 0;
}

void Usage (char* prog_name) {
    fprintf(stderr, "usage: %s <thread_count>\n", prog_name);
    exit(0);
}  /* Usage */


void Read_matrix(char* prompt, double A[], int m, int n) {
    int i, j;

    printf("%s\n", prompt);
    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            scanf("%lf", &A[i*n+j]);
}  /* Read_matrix */


void Read_vector(char* prompt, double x[], int n) {
    int i;

    printf("%s\n", prompt);
    for (i = 0; i < n; i++)
        scanf("%lf", &x[i]);
}  /* Read_vector */


void Gen_matrix(double A[], int m, int n) {
    int i, j;
    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            A[i*n+j] = rand()/((double) RAND_MAX);
}


void Gen_vector(double x[], int n) {
    int i;
    for (i = 0; i < n; i++)
        x[i] = rand()/((double) RAND_MAX);
}


void *Pth_mat_vect_shared(void* rank) {
    long my_rank = (intptr_t) rank;
    int i, j;
    int local_m = m/thread_count;
    int my_first_row = my_rank*local_m;
    int my_last_row = (my_rank+1)*local_m - 1;

    for (i = my_first_row; i <= my_last_row; i++) {
        y[i+(my_rank*8)] = 0.0;
        for (j = 0; j < n; j++)
            y[i+(my_rank*8)] += A[i*n+j]*x[j];
    }

    return NULL;
}

void *Pth_mat_vect_local(void* rank) {
    long my_rank = (intptr_t) rank;
    int i, j;
    int local_m = m/thread_count;
    register int sub = my_rank*local_m*n;
    int my_first_row = my_rank*local_m;
    int my_last_row = (my_rank+1)*local_m - 1;
    double tmp;

    for (i = my_first_row; i <= my_last_row; i++) {
        tmp = 0.0;
        for (j = 0; j < n; j++)
            tmp += A[sub++]*x[j];
        y[i] = tmp;
    }

    return NULL;
}


void Print_matrix( char* title, double A[], int m, int n) {
    int   i, j;

    printf("%s\n", title);
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++)
            printf("%4.1f ", A[i*n + j]);
        printf("\n");
    }
}


void Print_vector(char* title, double y[], double m) {
    int   i;

    printf("%s\n", title);
    for (i = 0; i < m; i++)
        printf("%4.1f ", y[i]);
    printf("\n");
}

