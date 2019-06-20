#include<iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>


void Generate_data(int a[], int n) {
   int i;
   
   for (i = 0; i < n; i++){
       srand((unsigned int)(time(NULL)));
       a[i] = rand() % n + 1; 
   } 
      
}


void Count_sort(int a[], int n) {
    int i, j, count;
    int * temp = (int*)malloc(n*sizeof(int));
    for(i=0; i<n; i++) {
        count = 0;
        for(j=0; j<n; j++) {
            if(a[j]<a[i])count++;
            else if(a[j]==a[i]&&j<i)count++;
            temp[count] = a[i];
        }
    }
    memcpy(a, temp, n*sizeof(int));
    free(temp);
}

void Count_sort_parallel(int a[], int n, int thread_count) {
    int i, j, count;
    int* temp = (int*)malloc(n*sizeof(int));

    #  pragma omp parallel num_threads(thread_count) default(none) \
    private(i, j, count) shared(n, a, temp)
    {
        #     pragma omp for
        for (i = 0; i < n; i++) {
            count = 0;
            for (j = 0; j < n; j++)
                if (a[j] < a[i])
                    count++;
                else if (a[j] == a[i] && j < i)
                    count++;
            temp[count] = a[i];
        }

        #     pragma omp for
        for (i = 0; i < n; i++)
            a[i] = temp[i];
    }

    free(temp);
}

int cmp(const void* a, const void* b) {
   const int* int_a = (const int*) a;
   const int* int_b = (const int*) b;
   
   return (*int_a - *int_b);
} 

void Qsort(int a[], int n) {
   qsort(a, n, sizeof(int), cmp);
}  


int main() {
    int n = 50000;
    int a[n];
    double start, end;
    int thread_count = 4;

    Generate_data(a, n);
    
    start = clock();
    Count_sort(a, n);
    end = clock();
    std::cout<<"Serial time: "<<end - start<<std::endl;
    
    Generate_data(a, n);
    start = clock();
    Count_sort_parallel(a, n, thread_count);
    end = clock();
    std::cout<<"Parallel("<<thread_count<<") time: "<<end - start<<std::endl;
    
    Generate_data(a, n);
    start = clock();
    Qsort(a, n);
    end = clock();
    std::cout<<"Qsort time: "<<end - start<<std::endl;

    return 0;
}

