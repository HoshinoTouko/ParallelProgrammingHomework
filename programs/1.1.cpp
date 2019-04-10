#include <cstdio>
#include <iostream>
#include <mpi.h>

MPI_Comm mpi_comm = MPI_COMM_WORLD;
const int MAX_VALUE = 20;
int rank, size;

int ring_pass_sum(int val);
int butterfly_sum(int val);

int main (int argc, char **argv)
{
  int val, sum;
  double start_time, end_time, my_consume, g_consume;

  MPI_Init (&argc, &argv);  /* starts MPI */

  MPI_Comm_rank (mpi_comm, &rank);    /* get current process id */
  MPI_Comm_size (mpi_comm, &size);    /* get number of processes */

  srandom(rank);
  val = random() % MAX_VALUE;

  std::cout<<"Hello world from process "<<rank<<" of "<<size<<" with value "<<val<<std::endl;

  /* Calculate ring pass sum */
  MPI_Barrier(mpi_comm);
  start_time = MPI_Wtime();
  sum = ring_pass_sum(val);
  end_time = MPI_Wtime();
  my_consume = end_time - start_time;
  MPI_Reduce(&my_consume, &g_consume, 1, MPI_DOUBLE, MPI_MAX, 0, mpi_comm);
  if (rank == 0){
    std::cout<<"Ring pass sum result is "<<sum<<std::endl;
    std::cout<<"Ring pass time usage is "<<g_consume<<std::endl;
  }

  /* Calculate butterfly sum */
  MPI_Barrier(mpi_comm);
  start_time = MPI_Wtime();
  sum = butterfly_sum(val);
  end_time = MPI_Wtime();
  my_consume = end_time - start_time;
  MPI_Reduce(&my_consume, &g_consume, 1, MPI_DOUBLE, MPI_MAX, 0, mpi_comm);
  if (rank == 0){
    std::cout<<"Butterfly sum result is "<<sum<<std::endl;
    std::cout<<"Butterfly time usage"<<g_consume<<std::endl;
  }

  MPI_Finalize();

  return 0;
}

int ring_pass_sum(int val){
  int source, dest, sum, temp_val;
  
  sum = val;
  temp_val = val;
  source = (rank + size - 1) % size;
  dest = (rank + 1) % size;
  
  for (int i = 1; i < size ; i++) {
    MPI_Sendrecv_replace(&temp_val, 1, MPI_INT, dest, 0, 
          source, 0, mpi_comm, MPI_STATUS_IGNORE);
    sum += temp_val;
  }
  
  return sum;
}

int butterfly_sum(int val){
  int partner, temp_val, sum;
  unsigned mask = 1;

  sum = val;
  temp_val = val;
  
  while (mask < size) {
    partner = rank ^ mask;
    MPI_Sendrecv(&sum,  1, MPI_INT, partner, 0,
                  &temp_val, 1, MPI_INT, partner, 0,
                  mpi_comm, MPI_STATUS_IGNORE);
    sum += temp_val;
    mask <<= 1;
  }

  return sum;
}
