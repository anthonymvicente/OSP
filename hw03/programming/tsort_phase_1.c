#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<errno.h>
#include"tsort.h"

void tsort(int, int);
void tswap(int, int);

#define BASE 10

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

    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }

    tsort(0, array_size - 1);

    for(i = 0; i < array_size; i++)
    {
        printf("input_array[%d]: %d\n", i, input_array[i]);
    }

    return 0;
}

void tsort(int b_index, int e_index)
{
    int sorted = 0;
    int c_index = 0;

    while(!sorted)
    {
        sorted = 1;
        for(c_index = b_index; c_index < e_index; c_index++)
        {
            if(input_array[c_index] > input_array[c_index + 1])
            {
                tswap(c_index, c_index + 1);
                sorted = 0;
            }
        }

        if(sorted)
        {
            break;
        }

        sorted = 1;
        for(c_index = e_index; c_index > b_index; c_index--)
        {
            if(input_array[c_index - 1] > input_array[c_index])
            {
                tswap(c_index, c_index - 1);
                sorted = 0;
            }
        }
    }

}

void tswap(int x, int y)
{
    int temp = input_array[x];
    input_array[x] = input_array[y];
    input_array[y] = temp;
}
