#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>

const int MAX_FILES = 50;
const int MAX_CHAR = 100;

struct list_node_s {
    char* data;
    struct list_node_s* next;
};

void Usage(char* prog_name);
void Prod_cons(int prod_count, int cons_count, FILE* files[], int file_count);
void Get_files(FILE* files[], int* file_count_p);
void Read_file(FILE* file, struct list_node_s** queue_head,
               struct list_node_s** queue_tail, int my_rank);
void Enqueue(char* line, struct list_node_s** queue_head,
             struct list_node_s** queue_tail);
struct list_node_s* Dequeue(struct list_node_s** queue_head,
                            struct list_node_s** queue_tail, int my_rank);
void Tokenize(char* data, int my_rank);
void Print_queue(int my_rank, struct list_node_s* queue_head);

int main(int argc, char* argv[]) {
    int   prod_count, cons_count;
    FILE* files[MAX_FILES];
    int   file_count;

    if (argc != 3) Usage(argv[0]);
    prod_count = strtol(argv[1], NULL, 10);
    cons_count = strtol(argv[2], NULL, 10);

    /* Read in list of filenames and open files */
    Get_files(files, &file_count);

    /* Producer-consumer */
    Prod_cons(prod_count, cons_count, files, file_count);

    return 0;
}

void Usage(char* prog_name) {

    fprintf(stderr, "usage: %s <producer count> <consumer count>\n",
            prog_name);
    exit(0);
}

void Get_files(FILE* files[], int* file_count_p) {
    int i = 0;
    char filename[MAX_CHAR];

    while (scanf("%s", filename) != -1) {
        files[i] = fopen(filename, "r");

        if (files[i] == NULL) {
            fprintf(stderr, "Can't open %s\n", filename);
            fprintf(stderr, "Quitting . . . \n");
            exit(-1);
        }
        i++;
    }
    *file_count_p = i;
}

void Prod_cons(int prod_count, int cons_count, FILE* files[],
               int file_count) {
    int thread_count = prod_count + cons_count;
    struct list_node_s* queue_head = NULL;
    struct list_node_s* queue_tail = NULL;
    int prod_done_count = 0;

    #  pragma omp parallel num_threads(thread_count) default(none) \
    shared(file_count, queue_head, queue_tail, files, prod_count, \
           cons_count, prod_done_count)
    {
        int my_rank = omp_get_thread_num(), f;
        if (my_rank < prod_count) {
            /* Producer code */
            /* A cyclic partition of the files among the producers */
            for (f = my_rank; f < file_count; f += prod_count) {
                Read_file(files[f], &queue_head, &queue_tail, my_rank);
            }
            #        pragma omp atomic
            prod_done_count++;
        } else {
            struct list_node_s* tmp_node;

            while (prod_done_count < prod_count) {
                tmp_node = Dequeue(&queue_head, &queue_tail, my_rank);
                if (tmp_node != NULL) {
                    Tokenize(tmp_node->data, my_rank);
                    free(tmp_node);
                }
            }

            while (queue_head != NULL) {
                tmp_node = Dequeue(&queue_head, &queue_tail, my_rank);
                if (tmp_node != NULL) {
                    Tokenize(tmp_node->data, my_rank);
                    free(tmp_node);
                }
            }
        }
    }
}

void Read_file(FILE* file, struct list_node_s** queue_head,
               struct list_node_s** queue_tail, int my_rank) {

    char* line = (char*)malloc(MAX_CHAR*sizeof(char));
    while (fgets(line, MAX_CHAR, file) != NULL) {
        printf("Th %d > read line: %s", my_rank, line);
        Enqueue(line, queue_head, queue_tail);
        line = (char*)malloc(MAX_CHAR*sizeof(char));
    }

    fclose(file);
}

void Enqueue(char* line, struct list_node_s** queue_head,
             struct list_node_s** queue_tail) {
    struct list_node_s* tmp_node = NULL;

    tmp_node = (list_node_s*)malloc(sizeof(struct list_node_s));
    tmp_node->data = line;
    tmp_node->next = NULL;

    #  pragma omp critical
    if (*queue_tail == NULL) { // list is empty
        *queue_head = tmp_node;
        *queue_tail = tmp_node;
    } else {
        (*queue_tail)->next = tmp_node;
        *queue_tail = tmp_node;
    }
}

struct list_node_s* Dequeue(struct list_node_s** queue_head,
                            struct list_node_s** queue_tail, int my_rank) {
    struct list_node_s* tmp_node = NULL;

    if (*queue_head == NULL) // empty
        return NULL;

    #  pragma omp critical
    {
        if (*queue_head == *queue_tail) // last node
            *queue_tail = (*queue_tail)->next;

        tmp_node = *queue_head;
        *queue_head = (*queue_head)->next;
    }

    return tmp_node;
}

void Tokenize(char* data, int my_rank) {
    char* my_token, *word;

    my_token = strtok_r(data, " \t\n", &word);
    while (my_token != NULL) {
        printf("Th %d token: %s\n", my_rank, my_token);
        my_token = strtok_r(NULL, " \t\n", &word);
    }
}

void Print_queue(int my_rank, struct list_node_s* queue_head) {
    struct list_node_s* curr_p = queue_head;

    printf("Th %d > queue = \n", my_rank);
    #  pragma omp critical
    while (curr_p != NULL) {
        printf("%s", curr_p->data);
        curr_p = curr_p->next;
    }
    printf("\n");
}
