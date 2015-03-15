#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<errno.h>
#include<pthread.h>
#include"tsort.h"

int *input_array;

int main(int argc, char *argv[])
{
    int thread_num = atoi(argv[1]);


    // READ FILE - BEGIN
    int array_size = 0;
    char f_line[MAX_BUFF];
    char *f_results;
    char *after_num;
    FILE *fp;
    char *file_path = argv[2];

    fp = fopen(file_path, "r");

    if(fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", file_path);
        exit(1);
    }

    f_results = fgets(f_line, MAX_BUFF, fp);

    if(f_results == NULL)
    {
        fprintf(stderr, "Empty file\n");
        exit(1);
    }

    array_size = strtoimax(f_line, &after_num, BASE);

    if(errno == ERANGE)
    {
        fprintf(stderr, "value out of range : %s\n", f_line);
        exit(1);
    }

    input_array = (int *) malloc(sizeof(int) * array_size);

    int i = 0;
    for(i = 0; i < array_size; i++)
    {
        f_results = fgets(f_line, MAX_BUFF, fp);

        if(f_results == NULL)
        {
            fprintf(stderr, "Improperly formatted file, expecting more lines.\n");
            exit(1);
        }

        input_array[i] = strtoimax(f_line, &after_num, BASE);

        if(errno == ERANGE)
        {
            fprintf(stderr, "value out of range : %s\n", f_line);
            exit(1);
        }
    }

    // READ FILE - END

    // calculate number of elements in sub arrays
    int sub_array_size = array_size/thread_num;
    // calculate remainder elements if array is not evenly divisible
    int sub_array_remainder = array_size % thread_num;

    printf("UNSORTED ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }
    printf("\n");

    //tsort(0, array_size - 1);

    // set up variables for thread
    int rc;
    pthread_t thread[thread_num];
    void *status;

    struct indices ind[thread_num];
    for(i = 0; i < thread_num; i++)
    {

        ind[i].thread_num = i;
        ind[i].b_index = i * sub_array_size;
        ind[i].e_index = ind[i].b_index + sub_array_size;
        if(i == thread_num - 1)
        {
            if(sub_array_remainder == 0)
            {
                ind[i].e_index--;
            } else
            {
                ind[i].e_index = ind[i].e_index + sub_array_remainder - 1;
            }
        }
        ind[i].array_size = sub_array_size;

        printf("t:%d b_index: %d\n", ind[i].thread_num, ind[i].b_index);
        printf("t:%d e_index: %d\n", ind[i].thread_num, ind[i].e_index);

        rc = pthread_create(&thread[i], NULL, tsort, (void *) &ind[i]);

    }


    for(i = 0; i < thread_num; i++)
    {
        pthread_join(thread[i], &status);
    }

    printf("SORTED ARRAY\n");
    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }
    printf("\n");

    return 0;
}
