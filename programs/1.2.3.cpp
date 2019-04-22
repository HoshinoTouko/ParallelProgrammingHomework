/*
 * Purpose:
 *      Parallelize 1.2.1 serial program designed on a system with n processes.
 *      Let n = 2^k and k be a positive integer. Design a parallel algorithm that 
 *      makes this parallel algorithm only require k communication stages
 * Compile:
 *      mpic++ 1.2.3.cpp
 * Execute:
 *      mpirun -n 32 ./a.out
 *      # Note that the number of process must be evenly divisible by VECTOR_SIZE and 
 *      n is the power of 2 m.
 */
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<mpi.h>

#define VECTOR_SIZE 3200

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

void calculate_vector(int loc_n, int loc_vector_storage[], int loc_prefix_sums[]){
    int passed_sum;
    int mask, sum, partner, tmp;

    // Compute self task
    loc_prefix_sums[0] = loc_vector_storage[0];
    for (int i = 1; i < loc_n; i++){
        loc_prefix_sums[i] = loc_prefix_sums[i - 1] + loc_vector_storage[i];
    }

    /* Now use butterfly structured communications */
    sum = loc_prefix_sums[loc_n-1];
    mask = 1;
    while (mask < size) {
        partner = rank ^ mask;
        MPI_Sendrecv(&sum, 1, MPI_DOUBLE, partner, 0,
                &tmp, 1, MPI_DOUBLE, partner, 0,
                mpi_comm, MPI_STATUS_IGNORE);
        sum += tmp;
        if (rank > partner) 
            for (int i = 0; i < loc_n; i++)
                loc_prefix_sums[i] += tmp;
        mask <<= 1;
    }
}

void print_vector(std::string title, int vector[], int vec_size){
    std::cout<<title<<std::endl;
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

    calculate_vector(loc_n, loc_vector_storage, loc_prefix_sums);

    // print_vector("loc_vector_storage", loc_vector_storage, loc_n);
    print_vector("loc_prefix_sums", loc_prefix_sums, loc_n);

    free(loc_vector_storage);
    free(loc_prefix_sums);

    MPI_Finalize();
    return 0;
}
