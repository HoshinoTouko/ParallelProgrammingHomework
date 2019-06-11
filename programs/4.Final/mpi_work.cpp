#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mpi.h>

int my_rank, comm_sz;
double dx;

double f(double x) {
  double return_val;

  return_val = x * x;
  return return_val;
}

double Trap(double a, double b, double local_n) {
    double h, x, my_result;
    double local_a, local_b;

    local_a = a + my_rank * local_n * dx;
    local_b = local_a + local_n * dx;
    my_result = (f(local_a) + f(local_b)) / 2.0;
    for (int i = 1; i < local_n; i++) {
        x = local_a + i * dx;
        my_result += f(x);
    }
    my_result *= dx;
    return my_result;
}

int main(int argc, char **argv) {
  
  int local_n;
  double local_a;
  double local_b;
  double total_inte;

  double a = 1.0;
  double b = 2.0;
  int n = 32000;

  double start, end;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

  std::cout << "Standard " << (b * b * b - a * a * a) / 3 << std::endl;

  local_n = n / comm_sz;
  dx = (b - a) / n;
  local_a = a + my_rank * local_n * dx;
  local_b = local_a + local_n * dx;

  start = clock();

  double local_result = Trap(local_a, local_b, local_n);
  double final_result = 0.0;

  if (my_rank != 0) {
    MPI_Send(&local_result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  } else {
    final_result = local_result;
    for (int source = 1; source < comm_sz; source++) {
      MPI_Recv(&local_result, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      final_result += local_result;
    }
  }

  end = clock();

  if (my_rank == 0) {
    std::cout << std::endl;
    std::cout << "Thread count: " << comm_sz << std::endl;
    std::cout << "CPU Time: " << end - start << ". Result: " << final_result
              << std::endl;
  }

  MPI_Finalize();

  return 0;
}
