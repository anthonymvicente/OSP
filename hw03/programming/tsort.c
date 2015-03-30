#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<errno.h>
#include<pthread.h>
#include"tsort.h"

int main(int argc, char *argv[])
{

    if(argc != 3)
    {
        fprintf(stderr, "usage: %s [thread number] [data file]\n", argv[0]);
        exit(1);
    }

    int thread_num = atoi(argv[1]);

    if(thread_num > 16 || thread_num < 1)
    {
        fprintf(stderr, "invalid number of threads\n");
        exit(1);
    }

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

    pthread_cond_t thread_conds[thread_num + 1];

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
        pthread_cond_init(&thread_conds[i], NULL);
    }
    pthread_cond_init(&thread_conds[thread_num], NULL);

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
        thread_params[i].thread_cond = &thread_conds[i];
        thread_params[i].main_cond = &thread_conds[thread_num];

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
            thread_params[i].l_cond = NULL;
            thread_params[i].r_cond = &thread_conds[i + 1];
        } else if(i == thread_num - 1)
        {
            thread_params[i].l_border = &border_locks[i - 1];
            thread_params[i].r_border = NULL;
            thread_params[i].l_cond = &thread_conds[i - 1];
            thread_params[i].r_cond = NULL;
        } else
        {
            thread_params[i].l_border = &border_locks[i - 1];
            thread_params[i].r_border = &border_locks[i];
            thread_params[i].l_cond = &thread_conds[i - 1];
            thread_params[i].r_cond = &thread_conds[i + 1];
        }

        thread_create_status = pthread_create(&thread[i], NULL, tsort, &thread_params[i]);

        if(thread_create_status != 0)
        {
            fprintf(stderr, "error creating thread\n");
            exit(1);
        }
    }

    int all_sorted = 0;
    int b_index = 0;
    int e_index = 0;
    int thread_wake = 0;

    pthread_mutex_lock(&state_lock);
    while(!all_sorted)
    {
        if(is_sorted(0, array_size - 1, input_array))
        {
            //printf("all sorted\n");
            all_sorted = 1;
            break;
        }
        //printf("main waiting\n");
        pthread_cond_wait(&thread_conds[thread_num], &state_lock);
        //printf("main waking\n");
        if(is_sorted(0, array_size - 1, input_array))
        {
            //printf("all sorted\n");
            all_sorted = 1;
        } else
        {
            for(i = 0; i < thread_num; i++)
            {
                b_index = thread_params[i].b_index;
                e_index = thread_params[i].e_index;
                if(!is_sorted(b_index, e_index, input_array))
                {
                    pthread_cond_signal(&thread_conds[i]);
                    thread_wake = 1;
                }
            }

            if(thread_wake)
            {
                //printf("thread woken\n");
            }
            thread_wake = 0;
        }

    }
    pthread_mutex_unlock(&state_lock);

    /*
    for(i = 0; i < thread_num; i++)
    {
        thread_join_status = pthread_join(thread[i], NULL);

        if(thread_join_status != 0)
        {
            fprintf(stderr, "error joining thread %d\n", i);
            exit(1);
        }
    }
    */

    //printf("SORTED ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        //printf("input_array[%d]: %d\n", i, input_array[i]);
        printf("%d\n", input_array[i]);
    }
    //printf("\n");

    return 0;
}
