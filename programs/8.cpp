#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <stdint.h>
#include <pthread.h>

#define ll long long int

const int thread_count = 8;
const ll number_of_tosses = 1000000000;
ll number_in_circle = 0;

pthread_mutex_t mutex;


void *Simulating_throw(void* rank);


int main(int argc, char* argv[]) {
    long thread;
    pthread_t* thread_handles;
    double pi_estimate;

    thread_handles = (pthread_t*)malloc(thread_count*sizeof(pthread_t));
    pthread_mutex_init(&mutex, NULL);
    
    std::cout<<"Simulate "<<number_of_tosses<<" times tosses with "<<thread_count<<" processes."<<std::endl;

    for(thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, Simulating_throw, (void*) thread);
    }

    for(thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    pi_estimate = 4 * number_in_circle / ((double) number_of_tosses);
    printf("Estimated pi: %e\n", pi_estimate);

    pthread_mutex_destroy(&mutex);
    return 0;
}


void *Simulating_throw(void* rank) {
    long my_rank = (intptr_t) rank;
    ll local_number_in_circle = 0;
    ll local_tosses = number_of_tosses/thread_count;
    ll start = local_tosses*my_rank;
    ll finish = start+local_tosses;
    double x, y, distance_squared;

    srand((unsigned)(++my_rank * time(NULL)));
    for(ll toss = start; toss < finish; toss++) {
        x = 2 * rand() / double(RAND_MAX) - 1;
        y = 2 * rand() / double(RAND_MAX) - 1;
        distance_squared = x * x + y * y;

        if (distance_squared <= 1)
            local_number_in_circle++;
    }
    
    // Mutex lock
    pthread_mutex_lock(&mutex);
    number_in_circle += local_number_in_circle;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

