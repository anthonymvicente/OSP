#include<inttypes.h>

#define MAX_BUFF 256
#define MISSING_PAGE 257
#define TLB_SIZE 16
#define PAGE_SIZE 256
#define NUM_PAGES 256
#define L_MASK 0xFFFF
#define P_MASK 0xFF00
#define P_SHIFT 8
#define O_MASK 0x00FF

typedef struct l_listX
{
    uint16_t val;
    struct l_listX *next;
} l_list;

typedef struct tlb_entryX
{
    uint8_t p_number;
    uint8_t f_number;
} tlb_entry;

int read_address_store(char *, l_list *);
void initialize_tlb();
tlb_entry *search_tlb(uint8_t);
void tlb_insert(int, uint8_t, uint8_t);
void initialize_page_tbl();
uint16_t page_tbl_fetch(uint8_t);
void page_tbl_insert(int, uint8_t);
char phys_mem_fetch(uint8_t, uint8_t);
void phys_mem_insert(int, char *);
void free_tlb();
void free_l_list(l_list *);
