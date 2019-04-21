/*
 * Purpose:
 *      Serial summation.
 * Compile:
 *      g++ 1.2.1.cpp
 * Execute:
 *      ./a.out
 */
#include<cstdio>
#include<ctime>
#include<iostream>

#define VECTOR_SIZE 1000

// Generate a vector to do sum operation.
void generate_vector(int vector[]);

void generate_vector(int vector[]){
    for(int i = 0; i < VECTOR_SIZE; i++){
        vector[i] = 10;
    }
}

int main(){
    int sum = 0;
    int vector[VECTOR_SIZE];
    generate_vector(vector);
    for(int i = 0; i < VECTOR_SIZE; i++){
        sum += vector[i];
    }
    std::cout<<sum<<std::endl;
    return 0;
}
