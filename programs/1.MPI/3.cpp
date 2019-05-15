/*
 * Purpose:
 *      Get a matrix from user's input and output the up & right of the matrix.
 *
 * Compile:
 *      mpic++ 3.cpp
 *
 * Execute:
 *      mpirun -n 2 ./a.out
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mpi.h>

using namespace std;

int my_rank, comm_sz;
MPI_Comm comm;

void Build_indexed_type(int n, MPI_Datatype *indexed_mpi_t_p) {
  int i;
  int *block_lens, *disp;

  block_lens = (int *)malloc(n * sizeof(int));
  disp = (int *)malloc(n * sizeof(int));

  for (i = 0; i < n; i++) {
    block_lens[i] = n - i;
    disp[i] = i * (n + 1);
  }

  MPI_Type_indexed(n, block_lens, disp, MPI_DOUBLE, indexed_mpi_t_p);
  MPI_Type_commit(indexed_mpi_t_p);

  free(block_lens);
  free(disp);
}

void Mat_input(double loc_mat[], int n) {
  int i, j;

  std::cout << "Enter matrix items." << std::endl;
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      std::cin >> loc_mat[i * n + j];
}

void Mat_print(double loc_mat[], int n) {
  int i, j;
  cout << "Process " << my_rank << ": " << endl;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
      cout<<loc_mat[i * n + j]<<" ";
    cout << endl;
  }
  cout << endl;
}

int main(void) {
  int n;
  double *loc_mat;
  MPI_Datatype indexed_mpi_t;

  MPI_Init(NULL, NULL);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);

  cout<<"Enter the order of the matrix"<<endl;
  cin>>n;

  loc_mat = (double *)malloc(n * n * sizeof(double));

  Build_indexed_type(n, &indexed_mpi_t);
  if (my_rank == 0) {
    Mat_input(loc_mat, n);

    MPI_Send(loc_mat, 1, indexed_mpi_t, 1, 0, comm);
  } else if (my_rank == 1) {
    memset(loc_mat, 0, n * n * sizeof(double));
    MPI_Recv(loc_mat, 1, indexed_mpi_t, 0, 0, comm, MPI_STATUS_IGNORE);

    cout << "Received matrix" << endl;
    Mat_print(loc_mat, n);
  }

  free(loc_mat);
  MPI_Type_free(&indexed_mpi_t);
  MPI_Finalize();
  return 0;
}
