#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<errno.h>
#include<pthread.h>
#include"tsort.h"

int *input_array;
pthread_mutex_t *border_lock;

int main(int argc, char *argv[])
{
    int thread_num = atoi(argv[1]);
    char *file_name = argv[2];

    int *input_array;

    int array_size = read_file(file_name, &input_array);

    // calculate number of elements in sub arrays
    int sub_array_size = array_size/thread_num;
    // calculate remainder elements if array is not evenly divisible
    int sub_array_remainder = array_size % thread_num;

    printf("UNSORTED ARRAY\n");
    int i;
    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }
    printf("\n");

    //tsort(0, array_size - 1);

    int thread_create_status;
    int thread_join_status;
    pthread_t thread;

    param_struct thread_params;
    thread_params.b_index = 0;
    thread_params.e_index = array_size - 1;
    thread_params.input_array = input_array;

    thread_create_status = pthread_create(&thread, NULL, tsort, &thread_params);

    if(thread_create_status != 0)
    {
        fprintf(stderr, "error creating thread\n");
        exit(1);
    }

    thread_join_status = pthread_join(thread, NULL);

    if(thread_join_status != 0)
    {
        fprintf(stderr, "error joining thread\n");
        exit(1);
    }

    printf("SORTED ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }
    printf("\n");

    return 0;
}
