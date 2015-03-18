#define MAX_BUFF 256
#define BASE 10

typedef struct param_structX
{
    int b_index;
    int e_index;
    int *input_array;
    pthread_mutex_t *l_border;
    pthread_mutex_t *r_border;
} param_struct;

int read_file(char *, int **);
void *tsort(void *);
void tswap(int, int, int *);
