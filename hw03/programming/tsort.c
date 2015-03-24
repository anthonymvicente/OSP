#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<errno.h>
#include<pthread.h>
#include"tsort.h"

int main(int argc, char *argv[])
{
    int thread_num = atoi(argv[1]);
    char *file_name = argv[2];

    int *input_array;
    int *key_array;

    int array_size = read_file(file_name, &input_array, &key_array);

    // calculate number of elements in sub arrays
    int sub_array_size = array_size/thread_num;
    // calculate remainder elements if array is not evenly divisible
    int sub_array_remainder = array_size % thread_num;

    int i = 0;
/*
    printf("UNSORTED ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }
    printf("\n");

    printf("KEY ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        printf("key_array[%d]: %d\n", i, key_array[i]);
    }
    printf("\n");
*/
    int thread_create_status;
    int thread_join_status;
    int sublist_states[thread_num];
    pthread_t thread[thread_num];
    pthread_mutex_t border_locks[thread_num - 1];
    pthread_mutex_t state_lock;
    param_struct thread_params[thread_num];

    // initalize mutexs, VERY IMPORTANT
    pthread_mutex_init(&border_locks[0], NULL);
    for(i = 1; i < thread_num - 1; i++)
    {
        pthread_mutex_init(&border_locks[i], NULL);
    }

    pthread_mutex_init(&state_lock, NULL);

    // initialize sublist states
    for(i = 0; i < thread_num; i++)
    {
        sublist_states[i] = 0;
    }

    for(i = 0; i < thread_num; i++)
    {
        thread_params[i].sublist_num = i;
        // get start and end indices for each threads subarray
        thread_params[i].b_index = i * sub_array_size;
        thread_params[i].e_index = thread_params[i].b_index + sub_array_size;
        thread_params[i].input_array = input_array;
        thread_params[i].num_of_sublists = thread_num;
        thread_params[i].sublist_states = sublist_states;
        thread_params[i].state_lock = &state_lock;

        // handle remainder indices
        if(i == thread_num - 1)
        {
            if(sub_array_remainder == 0)
            {
                thread_params[i].e_index--;
            } else
            {
                thread_params[i].e_index = thread_params[i].e_index + sub_array_remainder - 1;
            }
        }

        //printf("thread %d b_index %d e_index %d\n", i, thread_params[i].b_index, thread_params[i].e_index);

        if(i == 0)
        {
            thread_params[i].l_border = NULL;
            thread_params[i].r_border = &border_locks[i];
        } else if(i == thread_num - 1)
        {
            thread_params[i].l_border = &border_locks[i - 1];
            thread_params[i].r_border = NULL;
        } else
        {
            thread_params[i].l_border = &border_locks[i - 1];
            thread_params[i].r_border = &border_locks[i];
        }

        thread_create_status = pthread_create(&thread[i], NULL, tsort, &thread_params[i]);

        if(thread_create_status != 0)
        {
            fprintf(stderr, "error creating thread\n");
            exit(1);
        }
    }

    for(i = 0; i < thread_num; i++)
    {
        thread_join_status = pthread_join(thread[i], NULL);

        if(thread_join_status != 0)
        {
            fprintf(stderr, "error joining thread %d\n", i);
            exit(1);
        }
    }

    //printf("SORTED ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        //printf("input_array[%d]: %d\n", i, input_array[i]);
        printf("%d\n", input_array[i]);
    }
    //printf("\n");

    return 0;
}
