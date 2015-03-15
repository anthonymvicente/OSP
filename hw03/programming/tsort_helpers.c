#include<stdio.h>
#include<pthread.h>
#include"tsort.h"

extern int *input_array;

void *tsort(void *ind)
{
    printf("in pthread\n");
    int sorted = 0;
    int c_index = 0;

    struct indices a_ind = *(struct indices *) ind;

    int b_index = a_ind.b_index;
    int e_index = a_ind.e_index;

    printf("p-thread_num: %d\np-b_index: %d\np-e_index: %d\n", a_ind.thread_num, b_index, e_index);

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

    printf("sub_array sorted: t: %d\n", a_ind.thread_num);
    int i;
    for(i = b_index; i < a_ind.array_size; i++)
    {
        printf("t: %d input_array(sub)[%d]: %d\n", a_ind.thread_num, i, input_array[i]);
    }

    pthread_exit(NULL);

}

void tswap(int x, int y)
{
    int temp = input_array[x];
    input_array[x] = input_array[y];
    input_array[y] = temp;
}
