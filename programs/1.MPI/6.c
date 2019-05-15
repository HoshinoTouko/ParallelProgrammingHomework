/*
 * Purpose:  Merge Sort of an array of nonnegative ints
 *
 * Compile:
 *    mpicc ./6.c
 *
 * Usage:
 *    mpirun -n 32 ./a.out
 *
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define RANDOM_MAX 100

void Usage(char* program);
void InitSelfList(int local_A[], int n);
void PrintGlobalList(int global_A[], int global_n);
void PrintMainProcessList(int local_A[], int local_n, int rank);
void MergeSort(int local_A[], int local_B[], int* local_n_p);
void GenerateSelfList(int local_A[], int local_n, int my_rank);
int CompareFunction(const void* a_p, const void* b_p);

void LocalSort(int local_A[], int global_n, int* local_n_p, int my_rank, int p,
               MPI_Comm comm);
void PrintSelfList(int local_A[], int local_n, unsigned bitmask, int my_rank,
                   int p, MPI_Comm comm);
void Read_list(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm);

int main(int argc, char* argv[]) {
    int global_n = 320;

    int my_rank, p;
    int* local_A;
    int local_n;
    MPI_Comm comm;

    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &my_rank);

    local_n = global_n / p;

    local_A = (int*)malloc(global_n * sizeof(int));

    printf("Generating local list...");
    InitSelfList(local_A, global_n);

    GenerateSelfList(local_A, local_n, my_rank);
    PrintSelfList(local_A, local_n, 0, my_rank, p, comm);

    LocalSort(local_A, global_n, &local_n, my_rank, p, comm);

    if (my_rank == 0) PrintGlobalList(local_A, global_n);

    free(local_A);
    MPI_Finalize();

    return 0;
}

void InitSelfList(int A[], int n) {
    int i;

    for (i = 0; i < n; i++) A[i] = -1;
}

void GenerateSelfList(int local_A[], int local_n, int my_rank) {
    int i;

    srandom(my_rank + 1);
    for (i = 0; i < local_n; i++) local_A[i] = random() % RANDOM_MAX;
}

void Usage(char* program) {
    fprintf(stderr, "usage:  mpiexec -n <p> %s <g|i> <global_n>\n", program);
    fprintf(stderr, "   - p: the number of processes (a power of 2)\n");
    fprintf(stderr, "   - 'g': generate random, distributed list\n");
    fprintf(stderr, "   - 'i': user will input list on process 0\n");
    fprintf(stderr, "   - global_n: number of elements in global list");
    fprintf(stderr, " (must be evenly divisible by p)\n");
    fflush(stderr);
}

/*-------------------------------------------------------------------
 * Function:   Read_list
 * Purpose:    process 0 reads the list from stdin and scatters it
 *             to the other processes.
 * In args:    local_n, my_rank, p, comm
 * Out arg:    local_A
 */
void Read_list(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm) {
    int i;
    int* temp;

    if (my_rank == 0) {
        temp = (int*)malloc(p * local_n * sizeof(int));
        printf("Enter the elements of the list\n");
        for (i = 0; i < p * local_n; i++) scanf("%d", &temp[i]);
    }

    MPI_Scatter(temp, local_n, MPI_INT, local_A, local_n, MPI_INT, 0, comm);

    if (my_rank == 0) free(temp);
}

void PrintGlobalList(int A[], int n) {
    int i;

    printf("The list is:\n   ");
    for (i = 0; i < n; i++) printf("%d ", A[i]);
    printf("\n");
}

int CompareFunction(const void* a_p, const void* b_p) {
    int a = *((int*)a_p);
    int b = *((int*)b_p);

    if (a < b)
        return -1;
    else if (a == b)
        return 0;
    else
        return 1;
}

void LocalSort(int local_A[], int global_n, int* local_n_p, int my_rank, int p,
               MPI_Comm comm) {
    int partner;
    int done = 0;
    unsigned bitmask = 1;
    MPI_Status status;

    /* LocalSort local list using built-in quick LocalSort */
    qsort(local_A, *local_n_p, sizeof(int), CompareFunction);

    while (!done && bitmask < p) {
        partner = my_rank ^ bitmask;
        if (my_rank < partner) {
            MPI_Recv(local_A + *local_n_p, *local_n_p, MPI_INT, partner, 0,
                     comm, &status);
            MergeSort(local_A, local_A + *local_n_p, local_n_p);
            bitmask <<= 1;
        } else {
            MPI_Send(local_A, *local_n_p, MPI_INT, partner, 0, comm);
            done = 1;
        }
    }
}

void MergeSort(int local_A[], int local_B[], int* local_n_p) {
    int ai, bi, ci, i;
    int* C;
    int csize = 2 * (*local_n_p);

    C = (int*)malloc(csize * sizeof(int));
    ai = 0;
    bi = 0;
    ci = 0;
    while ((ai < *local_n_p) && (bi < *local_n_p)) {
        if (local_A[ai] <= local_B[bi]) {
            C[ci] = local_A[ai];
            ci++;
            ai++;
        } else {
            C[ci] = local_B[bi];
            ci++;
            bi++;
        }
    }

    if (ai >= *local_n_p)
        for (i = ci; i < csize; i++, bi++) C[i] = local_B[bi];
    else if (bi >= *local_n_p)
        for (i = ci; i < csize; i++, ai++) C[i] = local_A[ai];

    for (i = 0; i < csize; i++) local_A[i] = C[i];

    free(C);
    *local_n_p = 2 * (*local_n_p);
}

void PrintMainProcessList(int local_A[], int local_n, int rank) {
    int i;
    printf("%d: ", rank);
    for (i = 0; i < local_n; i++) printf("%d ", local_A[i]);
    printf("\n");
}

void PrintSelfList(int local_A[], int local_n, unsigned bitmask, int my_rank,
                   int p, MPI_Comm comm) {
    int* A;
    int q;
    MPI_Status status;
    int increment;

    if (bitmask == 0)
        increment = 1;
    else
        increment = 2 * bitmask;

    if (my_rank == 0) {
        A = (int*)malloc(local_n * sizeof(int));
        PrintMainProcessList(local_A, local_n, my_rank);
        for (q = increment; q < p; q += increment) {
            MPI_Recv(A, local_n, MPI_INT, q, 0, comm, &status);
            PrintMainProcessList(A, local_n, q);
        }
        free(A);
    } else {
        MPI_Send(local_A, local_n, MPI_INT, 0, 0, comm);
    }
}
