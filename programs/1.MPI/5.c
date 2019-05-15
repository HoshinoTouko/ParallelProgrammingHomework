/* 
 * Purpose:  Determine message-passing times using ping pong.
 *
 * Compile:  mpicc ./5.c
 * 
 * Run:      mpiexec -n 2 ./a.out
 *
 * Usage: Change the definition of clock to use 
 *   MPI_Wtick or built-in clock func.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"

// #define CLOCK 1

#define WARMUP_ITERS 10
#define RES_TEST_ITERS 10
#define PING_PONG_ITERS 1000
#define MIN_MESG_SIZE 1
#define MAX_MESG_SIZE 131072
#define INCREMENT 1024
#define TEST_COUNT 1

const double clocks_per_sec = (double)CLOCKS_PER_SEC;

double ping_pong(char mesg[], int mesg_size, int iters, MPI_Comm comm, int p,
                 int my_rank);

/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    int test, mesg_size, i;
    double elapsed;
    double times[TEST_COUNT];
    char message[MAX_MESG_SIZE];
    char c = 'B';
    MPI_Comm comm;
    int p;
    int my_rank;

    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &my_rank);
    if (p != 2) {
        if (my_rank == 0) fprintf(stderr, "Use two processes\n");
        MPI_Finalize();
        return 0;
    }

    if (my_rank == 0) c = 'A';
    for (i = 0; i < MAX_MESG_SIZE; i++) message[i] = c;

    elapsed = ping_pong(message, MAX_MESG_SIZE, WARMUP_ITERS, comm, p, my_rank);

    elapsed = ping_pong(message, 0, RES_TEST_ITERS, comm, p, my_rank);
    if (my_rank == 0)
#ifndef CLOCK
        fprintf(stderr, "Use MPI_Wtick. \nMin ping_pong = %8.5e, Clock tick = %8.5e\n",
                elapsed / (2 * RES_TEST_ITERS), MPI_Wtick());
#else
        fprintf(stderr, "Use Clock. \nMin ping_pong = %8.5e, Clock tick = %8.5e\n",
                elapsed / (2 * RES_TEST_ITERS), 1.0 / clocks_per_sec);
#endif

    for (mesg_size = MIN_MESG_SIZE; mesg_size <= MAX_MESG_SIZE;
         mesg_size *= 2) {
        for (test = 0; test < TEST_COUNT; test++) {
            times[test] = ping_pong(message, mesg_size, PING_PONG_ITERS, comm,
                                    p, my_rank);
        } /* for test */

        if (my_rank == 0) {
            for (test = 0; test < TEST_COUNT; test++)
                printf("%d %8.5e\n", mesg_size,
                       times[test] / (2 * PING_PONG_ITERS));
        }
    } /* for mesg_size */

    MPI_Finalize();
    return 0;
} /* main */

/*-------------------------------------------------------------------*/
double ping_pong(char mesg[], int mesg_size, int iters, MPI_Comm comm, int p,
                 int my_rank) {
    int i;
    MPI_Status status;
    double start;

    if (my_rank == 0) {
#ifndef CLOCK
        start = MPI_Wtime();
#else
        start = clock() / clocks_per_sec;
#endif
        for (i = 0; i < iters; i++) {
            MPI_Send(mesg, mesg_size, MPI_CHAR, 1, 0, comm);
            MPI_Recv(mesg, mesg_size, MPI_CHAR, 1, 0, comm, &status);
        }
#ifndef CLOCK
        return MPI_Wtime() - start;
#else
        return clock() / clocks_per_sec - start;
#endif
    } else if (my_rank == 1) {
        for (i = 0; i < iters; i++) {
            MPI_Recv(mesg, mesg_size, MPI_CHAR, 0, 0, comm, &status);
            MPI_Send(mesg, mesg_size, MPI_CHAR, 0, 0, comm);
        }
    }
    return 0.0;
}
