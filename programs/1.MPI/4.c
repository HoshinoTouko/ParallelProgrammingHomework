/*
 * File:     prog3.5_mpi_mat_vect_col.c
 *
 * Purpose:  Implement matrix vector multiplication when the matrix
 *           has a block column distribution
 *
 * Compile:  mpicc 4.c
 *
 * Run:      mpiexec -n <comm_size> ./a.out
 *
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void GetDimension(int *m_p, int *local_m_p, int *n_p, int *local_n_p,
                  int cur_rank, int comm_size, MPI_Comm mpi_comm);
void AllocateArrays(double **local_A_pp, double **local_x_pp,
                    double **local_y_pp, int m, int local_m, int local_n,
                    MPI_Comm mpi_comm);
void BuildDerivedType(int m, int local_m, int n, int local_n,
                      MPI_Datatype *block_col_mpi_t_p);
void InputMatrix(char prompt[], double local_A[], int m, int local_n, int n,
                 MPI_Datatype block_col_mpi_t, int cur_rank, MPI_Comm mpi_comm);
void PrintNewMatrix(char title[], double local_A[], int m, int local_n, int n,
                    MPI_Datatype block_col_mpi_t, int cur_rank, MPI_Comm mpi_comm);
void InputVector(char prompt[], double local_vec[], int n, int local_n,
                 int cur_rank, MPI_Comm mpi_comm);
void PrintNewVector(char title[], double local_vec[], int n, int local_n,
                    int cur_rank, MPI_Comm mpi_comm);
void DoMainProcess(double local_A[], double local_x[], double local_y[],
                   int local_m, int m, int n, int local_n, int comm_size,
                   MPI_Comm mpi_comm);

int main(void) {
    double *local_A;
    double *local_x;
    double *local_y;

    int m, n;
    int local_m, local_n;

    int cur_rank, comm_size;
    MPI_Comm mpi_comm;
    MPI_Datatype block_col_mpi_t;

    MPI_Init(NULL, NULL);
    mpi_comm = MPI_COMM_WORLD;
    MPI_Comm_size(mpi_comm, &comm_size);
    MPI_Comm_rank(mpi_comm, &cur_rank);

    GetDimension(&m, &local_m, &n, &local_n, cur_rank, comm_size, mpi_comm);

    AllocateArrays(&local_A, &local_x, &local_y, m, local_m, local_n, mpi_comm);

    BuildDerivedType(m, local_m, n, local_n, &block_col_mpi_t);

    InputMatrix("Original Matrix", local_A, m, local_n, n, block_col_mpi_t, cur_rank, mpi_comm);

    InputVector("Original Vector", local_x, n, local_n, cur_rank, mpi_comm);

    DoMainProcess(local_A, local_x, local_y, local_m, m, n, local_n, comm_size,
                  mpi_comm);

    PrintNewVector("Result Vector", local_y, m, local_m, cur_rank, mpi_comm);

    free(local_A);
    free(local_x);
    free(local_y);
    MPI_Type_free(&block_col_mpi_t);
    MPI_Finalize();
    return 0;
}

void GetDimension(int *m_p, int *local_m_p, int *n_p, int *local_n_p,
                  int cur_rank, int comm_size, MPI_Comm mpi_comm) {
    if (cur_rank == 0) {
        printf("Enter the matrix order.\n");
        scanf("%d", m_p);
    }
    MPI_Bcast(m_p, 1, MPI_INT, 0, mpi_comm);
    *n_p = *m_p;

    *local_m_p = *m_p / comm_size;
    *local_n_p = *n_p / comm_size;
}

void AllocateArrays(double **local_A_pp, double **local_x_pp,
                    double **local_y_pp, int m, int local_m, int local_n,
                    MPI_Comm mpi_comm) {
    *local_A_pp = malloc(m * local_n * sizeof(double));
    *local_x_pp = malloc(local_n * sizeof(double));
    *local_y_pp = malloc(local_m * sizeof(double));
}

void BuildDerivedType(int m, int local_m, int n, int local_n,
                      MPI_Datatype *block_col_mpi_t_p) {
    MPI_Datatype vect_mpi_t;

    MPI_Type_vector(m, local_n, n, MPI_DOUBLE, &vect_mpi_t);
    MPI_Type_create_resized(vect_mpi_t, 0, local_n * sizeof(double),
                            block_col_mpi_t_p);
    MPI_Type_commit(block_col_mpi_t_p);
}

void InputMatrix(char prompt[], double local_A[], int m, int local_n, int n,
                 MPI_Datatype block_col_mpi_t, int cur_rank, MPI_Comm mpi_comm) {
    double *A = NULL;
    int local_ok = 1;
    int i, j;

    if (cur_rank == 0) {
        A = malloc(m * n * sizeof(double));

        printf("Enter the matrix %s\n", prompt);
        for (i = 0; i < m; i++)
            for (j = 0; j < n; j++) scanf("%lf", &A[i * n + j]);

        MPI_Scatter(A, 1, block_col_mpi_t, local_A, m * local_n, MPI_DOUBLE, 0,
                    mpi_comm);
        free(A);
    } else {
        MPI_Scatter(A, 1, block_col_mpi_t, local_A, m * local_n, MPI_DOUBLE, 0,
                    mpi_comm);
    }
}

void PrintNewMatrix(char title[], double local_A[], int m, int local_n, int n,
                    MPI_Datatype block_col_mpi_t, int cur_rank, MPI_Comm mpi_comm) {
    double *A = NULL;
    int i, j;

    if (cur_rank == 0) {
        A = malloc(m * n * sizeof(double));

        MPI_Gather(local_A, m * local_n, MPI_DOUBLE, A, 1, block_col_mpi_t, 0,
                   mpi_comm);

        printf("The matrix %s\n", title);
        for (i = 0; i < m; i++) {
            for (j = 0; j < n; j++) printf("%.2f ", A[i * n + j]);
            printf("\n");
        }

        free(A);
    } else {
        MPI_Gather(local_A, m * local_n, MPI_DOUBLE, A, 1, block_col_mpi_t, 0,
                   mpi_comm);
    }
}

void InputVector(char prompt[], double local_vec[], int n, int local_n,
                 int cur_rank, MPI_Comm mpi_comm) {
    double *vec = NULL;
    int i;

    if (cur_rank == 0) {
        vec = malloc(n * sizeof(double));
        printf("Enter the vector %s\n", prompt);
        for (i = 0; i < n; i++) scanf("%lf", &vec[i]);
        MPI_Scatter(vec, local_n, MPI_DOUBLE, local_vec, local_n, MPI_DOUBLE, 0,
                    mpi_comm);
        free(vec);
    } else {
        MPI_Scatter(vec, local_n, MPI_DOUBLE, local_vec, local_n, MPI_DOUBLE, 0,
                    mpi_comm);
    }
}

void DoMainProcess(double local_A[], double local_x[], double local_y[],
                   int local_m, int m, int n, int local_n, int comm_size,
                   MPI_Comm mpi_comm) {
    double *my_y;
    int *recv_counts;
    int i, loc_j;

    recv_counts = malloc(comm_size * sizeof(int));
    my_y = malloc(m * sizeof(double));

    for (i = 0; i < m; i++) {
        my_y[i] = 0.0;
        for (loc_j = 0; loc_j < local_n; loc_j++)
            my_y[i] += local_A[i * local_n + loc_j] * local_x[loc_j];
    }

    for (i = 0; i < comm_size; i++) {
        recv_counts[i] = local_m;
    }

    MPI_Reduce_scatter(my_y, local_y, recv_counts, MPI_DOUBLE, MPI_SUM, mpi_comm);

    free(my_y);
}

void PrintNewVector(char title[], double local_vec[], int n, int local_n,
                    int cur_rank, MPI_Comm mpi_comm) {
    double *vec = NULL;
    int i;

    if (cur_rank == 0) {
        vec = malloc(n * sizeof(double));
        MPI_Gather(local_vec, local_n, MPI_DOUBLE, vec, local_n, MPI_DOUBLE, 0,
                   mpi_comm);
        printf("\nPrint Vector: %s\n", title);
        for (i = 0; i < n; i++) printf("%f ", vec[i]);
        printf("\n");
        free(vec);
    } else {
        MPI_Gather(local_vec, local_n, MPI_DOUBLE, vec, local_n, MPI_DOUBLE, 0,
                   mpi_comm);
    }
}
