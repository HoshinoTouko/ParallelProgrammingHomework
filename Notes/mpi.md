# MPICH Note

## MPI Scatter

Watch https://www.mpich.org/static/docs/v3.1/www3/MPI_Scatter.html

This function is not share the data in the buf to all the process. What it will do is to send the data in it one by one to the rest of the processes in the order of the processes' rank. Here is an example.

Rank 0 send a list [0, 1, 2, 3, 4, 5, 6, 7] to 3 processes, for each process receive 4. 
Rank 0 will get result [0, 1, 2, 3]
Rank 1 will receive [4, 5, 6, 7]
Rank 2 and 3 will receive nothing.
