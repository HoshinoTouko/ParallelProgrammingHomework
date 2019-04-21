/*
 * Purpose:
 *      Parallelize 1.2.1 serial program designed on a system with n processes, 
 *      with each process storing an (x) value.
 * Compile:
 *      mpic++ 1.2.2.cpp
 * Execute:
 *      mpirun -n 50 ./a.out
 */
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include <mpi.h>

#define VECTOR_SIZE 100

int rank, size;
MPI_Comm mpi_comm = MPI_COMM_WORLD;

int data_size;


// Generate a vector to do sum operation.
void generate_vector(int vector[], int loc_n){
    int n = VECTOR_SIZE;
    int* tmp = NULL;

    if (rank == 0) {
        tmp = (int*)malloc(n * sizeof(int));
        for(int i = 0; i < n; i++)
            tmp[i] = 10;
        MPI_Scatter(tmp, loc_n, MPI_INT, vector, loc_n, MPI_INT, 0, mpi_comm);
        free(tmp);
   } else {
        MPI_Scatter(tmp, loc_n, MPI_INT, vector, loc_n, MPI_INT, 0, mpi_comm);
   }
}

void print_vector(int vector[], int vec_size){
    std::cout<<"Greeting from rank: "<<rank<<"[";
    for(int i = 0; i < vec_size; i++){
        std::cout<<vector[i]<<", ";
    }
    std::cout<<"]"<<std::endl;
}

int main(int argc, char* argv[]){
    int loc_n;
    int *loc_vector_storage, *loc_prefix_sums;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(mpi_comm, &rank);
    MPI_Comm_size(mpi_comm, &size);

    data_size = VECTOR_SIZE;
    loc_n = data_size / size;

    loc_vector_storage = (int*)malloc(loc_n * sizeof(int));
    loc_prefix_sums = (int*)malloc(loc_n * sizeof(int));

    generate_vector(loc_vector_storage, loc_n);

    print_vector(loc_vector_storage, loc_n);

    MPI_Finalize();
    return 0;
}
