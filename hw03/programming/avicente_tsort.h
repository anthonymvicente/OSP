#define MAX_BUFF 256
#define BASE 10

typedef struct param_structX
{
    int sublist_num;
    int b_index;
    int e_index;
    int *input_array;
    pthread_mutex_t *l_border;
    pthread_mutex_t *r_border;
    int num_of_sublists;
    int *sublist_states;
    pthread_mutex_t *state_lock;
    pthread_cond_t *thread_cond;
    pthread_cond_t *l_cond;
    pthread_cond_t *r_cond;
    pthread_cond_t *main_cond;
} param_struct;

int read_file(char *, int **, int **);
void *tsort(void *);
void tswap(int, int, int *);
int is_sorted(int , int , int *);
int check_states(int *, int);
void print_states(int *, int);
