#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


const int thread_count = 8;

double  a, b, h;
int     n, local_n;
int	method;


int flag;
sem_t sem;
pthread_mutex_t mutex;
double  res;

void *Thread_work(void* rank);
double Trap(double local_a, double local_b, int local_n,
            double h);    /* Calculate local integral  */
double f(double x); /* function we're integrating */


int main(int argc, char** argv) {
    double start, end;
    char labels[3][10] = {"Mutex", "Semaphore", "Busy-wait"};
    pthread_t*  thread_handles;

    res = 0.0;

    std::cout<<"Input a, b, n\n"<<std::endl;
    std::cin>>a>>b>>n;
    
    h = (b - a) / n;

    local_n = n / thread_count;

    thread_handles = (pthread_t*)malloc (thread_count*sizeof(pthread_t));

    flag = 0; // semaphore
    pthread_mutex_init(&mutex, NULL); // mutex
    sem_init(&sem, 0, 1); // busy-wait

    
    std::cout<<"Standard "<<(b * b * b - a * a * a) / 3<<std::endl;
    
    for (method = 0; method < 3; method++) {
        res = 0.0;
        start = clock();
        /* Start the threads. */
        for (long thread = 0; thread < thread_count; thread++) {
            pthread_create(&thread_handles[thread], NULL, Thread_work,
                           (void*) thread);
        }

        /* Wait for threads to complete. */
        for (long thread = 0; thread < thread_count; thread++) {
            pthread_join(thread_handles[thread], NULL);
        }
        end = clock();
        
        std::cout<<"Method: "<<labels[method]<<". CPU Time: "<<end - start<<". Result: "<<res<<std::endl;
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem);
    free(thread_handles);

    return 0;
}


void *Thread_work(void* rank) {
    double      local_a;   /* Left endpoint my thread   */
    double      local_b;   /* Right endpoint my thread  */
    double      my_int;    /* Integral over my interval */
    long        my_rank = (intptr_t) rank;


    local_a = a + my_rank * local_n * h;
    local_b = local_a + local_n * h;

    my_int = Trap(local_a, local_b, local_n, h);

    switch (method) {
        case 0: // Mutex
            pthread_mutex_lock(&mutex);
            res += my_int;
            pthread_mutex_unlock(&mutex);
            break;
        case 1: // Semaphore
            sem_wait(&sem);
            res += my_int;
            sem_post(&sem);
            break;
        case 2: // busy-wait
            while(flag != my_rank);
            res += my_int;
            flag = (flag+1) % thread_count;
            break;
        default:
            break;
    }

    return NULL;

}


double Trap(
    double  local_a   /* in */,
    double  local_b   /* in */,
    int     local_n   /* in */,
    double  h         /* in */) {

    double integral;   /* Store result in integral  */
    double x;
    int i;

    integral = (f(local_a) + f(local_b))/2.0;
    x = local_a;
    for (i = 1; i <= local_n-1; i++) {
        x = local_a + i*h;
        integral += f(x);
    }
    integral = integral*h;
    return integral;
}


double f(double x) {
    double return_val;

    return_val = x * x;
    return return_val;
}

