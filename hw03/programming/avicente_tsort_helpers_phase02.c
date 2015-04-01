#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<errno.h>
#include<pthread.h>
#include"avicente_tsort.h"

int read_file(char *file_name, int **input_array, int **key_array)
{
    int array_size = 0;
    char f_line[MAX_BUFF];
    char *f_results;
    char *after_num;
    FILE *fp;

    fp = fopen(file_name, "r");

    if(fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", file_name);
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

    *input_array = (int *) malloc(sizeof(int) * array_size);

    int i = 0;
    for(i = 0; i < array_size; i++)
    {
        f_results = fgets(f_line, MAX_BUFF, fp);

        if(f_results == NULL)
        {
            fprintf(stderr, "Improperly formatted file, expecting more lines.\n");
            exit(1);
        }

        (*input_array)[i] = strtoimax(f_line, &after_num, BASE);

        if(errno == ERANGE)
        {
            fprintf(stderr, "value out of range : %s\n", f_line);
            exit(1);
        }
    }

    f_results = fgets(f_line, MAX_BUFF, fp);

    // more data in file
    // assuming its the sorted array
    if(f_results != NULL)
    {
        *key_array = (int *) malloc(sizeof(int) * array_size);

        for(i = 0; i < array_size; i++)
        {
            (*key_array)[i] = strtoimax(f_line, &after_num, BASE);

            if(errno == ERANGE)
            {
                fprintf(stderr, "value out of range : %s\n", f_line);
                exit(1);
            }

            f_results = fgets(f_line, MAX_BUFF, fp);

            if(f_results == NULL && i != array_size - 1)
            {
                fprintf(stderr, "Improperly formatted file, expecting more lines.\n");
                exit(1);
            }
        }

    }

    return array_size;
}

void *tsort(void *params)
{
    int sorted = 0;
    int c_index = 0;

    param_struct *thread_params = (param_struct *) params;

    int b_index = thread_params->b_index;
    int e_index = thread_params->e_index;
    int *input_array = thread_params->input_array;
    pthread_mutex_t *l_border = thread_params->l_border;
    pthread_mutex_t *r_border = thread_params->r_border;
    pthread_mutex_t *state_lock = thread_params->state_lock;

    while(!sorted)
    {
        sorted = 1;
        for(c_index = b_index; c_index < e_index; c_index++)
        {
            // on the left side of the array, need to lock
            if(c_index == b_index && l_border != NULL)
            {
            //    printf("t: %d-%d locking at %d\n", b_index, e_index, c_index);
                pthread_mutex_lock(l_border);
            // on the right side of the array, need to lock
            } else if(c_index == e_index - 1 && r_border != NULL)
            {
            //    printf("t: %d-%d locking at %d\n", b_index, e_index, c_index);
                pthread_mutex_lock(r_border);
            }

            // swap elements and update state if needed
            if(input_array[c_index] > input_array[c_index + 1])
            {
                tswap(c_index, c_index + 1, input_array);
                sorted = 0;
                pthread_mutex_lock(state_lock);
                thread_params->sublist_states[thread_params->sublist_num] = 0;
                pthread_mutex_unlock(state_lock);

                // swapping at left border
                if(c_index == b_index && l_border != NULL)
                {
                //    printf("t: %d-%d borderswap left\n", b_index, e_index);
                    pthread_mutex_lock(state_lock);
                    thread_params->sublist_states[thread_params->sublist_num - 1] = 0;
                    pthread_mutex_unlock(state_lock);
                } else if(c_index == e_index - 1 && r_border != NULL)
                {
                //    printf("t: %d-%d borderswap right\n", b_index, e_index);
                    pthread_mutex_lock(state_lock);
                    thread_params->sublist_states[thread_params->sublist_num + 1] = 0;
                    pthread_mutex_unlock(state_lock);
                }
            }

            if(c_index == b_index && l_border != NULL)
            {
            //    printf("t: %d-%d unlocking at %d\n", b_index, e_index, c_index);
                pthread_mutex_unlock(l_border);
            } else if(c_index == e_index - 1 && r_border != NULL)
            {
            //    printf("t: %d-%d unlocking at %d\n", b_index, e_index, c_index);
                pthread_mutex_unlock(r_border);
            }
        }
        // ensure outside state matches inside state
        //thread_params->sublist_states[thread_params->sublist_num] = sorted;

        //printf("t: %d-%d sorted: %d sublist-state: %d\n", b_index, e_index, sorted, thread_params->sublist_states[thread_params->sublist_num]);

        if(sorted)
        {
            pthread_mutex_lock(state_lock);
            if(is_sorted(b_index, e_index, input_array))
            {
                thread_params->sublist_states[thread_params->sublist_num] = 1;
            } else
            {
                thread_params->sublist_states[thread_params->sublist_num] = 0;
                sorted = 0;
            }
            pthread_mutex_unlock(state_lock);
        }

        while(sorted)
        {
            //printf("t: %d-%d busy wait\n", b_index, e_index);
            pthread_mutex_lock(state_lock);
            if(check_states(thread_params->sublist_states, thread_params->num_of_sublists))
            {
                if(is_sorted(b_index, e_index, input_array))
                {
                    pthread_mutex_unlock(state_lock);
                    pthread_exit(NULL);
                } else
                {
                    thread_params->sublist_states[thread_params->sublist_num] = 0;
                }
            }
            sorted = thread_params->sublist_states[thread_params->sublist_num];
            pthread_mutex_unlock(state_lock);
        }

        sorted = 1;
        for(c_index = e_index; c_index > b_index; c_index--)
        {
            // on right side of array, lock
            if(c_index == e_index && r_border != NULL)
            {
            //    printf("t: %d-%d locking at %d\n", b_index, e_index, c_index);
                pthread_mutex_lock(r_border);
            // on left side of array, lock
            } else if(c_index == b_index + 1 && l_border != NULL)
            {
            //    printf("t: %d-%d locking at %d\n", b_index, e_index, c_index);
                pthread_mutex_lock(l_border);
            }
            if(input_array[c_index - 1] > input_array[c_index])
            {
                tswap(c_index, c_index - 1, input_array);
                sorted = 0;
                pthread_mutex_lock(state_lock);
                thread_params->sublist_states[thread_params->sublist_num] = 0;
                pthread_mutex_unlock(state_lock);

                if(c_index == e_index && r_border != NULL)
                {
                //    printf("t: %d-%d borderswap right\n", b_index, e_index);
                    pthread_mutex_lock(state_lock);
                    thread_params->sublist_states[thread_params->sublist_num + 1] = 0;
                    pthread_mutex_unlock(state_lock);
                } else if(c_index == b_index + 1 && l_border != NULL)
                {
                //    printf("t: %d-%d borderswap left\n", b_index, e_index);
                    pthread_mutex_lock(state_lock);
                    thread_params->sublist_states[thread_params->sublist_num - 1] = 0;
                    pthread_mutex_unlock(state_lock);
                }
            }
            if(c_index == e_index && r_border != NULL)
            {
            //    printf("t: %d-%d unlocking at %d\n", b_index, e_index, c_index);
                pthread_mutex_unlock(r_border);
            } else if(c_index == b_index + 1 && l_border != NULL)
            {
            //    printf("t: %d-%d unlocking at %d\n", b_index, e_index, c_index);
                pthread_mutex_unlock(l_border);
            }
        }
        //thread_params->sublist_states[thread_params->sublist_num] = sorted;

        if(sorted)
        {
            pthread_mutex_lock(state_lock);
            if(is_sorted(b_index, e_index, input_array))
            {
                thread_params->sublist_states[thread_params->sublist_num] = 1;
            } else
            {
                thread_params->sublist_states[thread_params->sublist_num] = 0;
                sorted = 0;
            }
            pthread_mutex_unlock(state_lock);
        }

        while(sorted)
        {
            //printf("t: %d-%d busy wait\n", b_index, e_index);
            pthread_mutex_lock(state_lock);
            if(check_states(thread_params->sublist_states, thread_params->num_of_sublists))
            {
                if(is_sorted(b_index, e_index, input_array))
                {
                    pthread_mutex_unlock(state_lock);
                    pthread_exit(NULL);
                } else
                {
                    thread_params->sublist_states[thread_params->sublist_num] = 0;
                }
            }
            sorted = thread_params->sublist_states[thread_params->sublist_num];
            pthread_mutex_unlock(state_lock);
        }

    }

    //printf("t: %d-%d exiting with state %d\n", b_index, e_index, thread_params->sublist_states[thread_params->sublist_num]);
    pthread_exit(NULL);
}

void tswap(int x, int y, int *input_array)
{
    int temp = input_array[x];
    input_array[x] = input_array[y];
    input_array[y] = temp;
}

int is_sorted(int b_index, int e_index, int *input_array)
{
    int i;
    for(i = b_index; i < e_index - 1; i++)
    {
        if(input_array[i] > input_array[i+1])
        {
            return 0;
        }
    }

    return 1;
}

int check_states(int *sublist_states, int num_of_sublists)
{
    int i = 0;
    for(; i < num_of_sublists; i++)
    {
        if(sublist_states[i] != 1)
        {
            return 0;
        }
    }

    return 1;
}

void print_states(int *sublist_states, int sublist_num)
{
    int i = 0;
    for(; i < sublist_num; i++)
    {
        printf("state %d: %d\n", i, sublist_states[i]);
    }
}
