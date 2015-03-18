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
    pthread_t thread[thread_num];
    param_struct thread_params[thread_num];

    for(i = 0; i < thread_num; i++)
    {
        // get start and end indices for each threads subarray
        thread_params[i].b_index = i * sub_array_size;
        thread_params[i].e_index = thread_params[i].b_index + sub_array_size;
        thread_params[i].input_array = input_array;

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

    printf("SORTED ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }
    printf("\n");

    return 0;
}
