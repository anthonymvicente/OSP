#define MAX_BUFF 256
#define BASE 10

struct indices
{
    int thread_num;
    int b_index;
    int e_index;
    int array_size;
};

void *tsort(void *);
void tswap(int, int);
