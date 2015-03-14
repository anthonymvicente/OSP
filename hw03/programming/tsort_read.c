#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<inttypes.h>
#include"tsort.h"

#define BASE 10

int read_tsort_file(char *file_path, int *input_array)
{
    int array_size = 0;
    char f_line[MAX_BUFF];
    char *f_results;
    char *after_num;
    FILE *fp;

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

    input_array = (int *) realloc(input_array, sizeof(int) * array_size);

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

    for(i = 0; i < array_size; i++)
    {
        printf("read input_array[%d]: %d\n", i, input_array[i]);
    }

    printf("closing\n");

    fclose(fp);

    printf("returning\n");

    return array_size;
}
