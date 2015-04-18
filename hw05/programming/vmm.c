#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include"vmm.h"

tlb_entry *tlb[TLB_SIZE];
uint16_t page_tbl[NUM_PAGES];
char phys_mem[NUM_PAGES * PAGE_SIZE];

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "usage: %s [backing store] [addresses]\n", argv[0]);
        return -1;
    }

    int tlb_hits = 0;
    int page_faults = 0;

    char *backing_store = argv[1];
    char *address_store = argv[2];

    FILE *bs_fp;
    char f_line[MAX_BUFF];

    l_list head;

    initialize_page_tbl();
    initialize_tlb();

    int num_of_addresses = read_address_store(address_store, &head);

    bs_fp = fopen(backing_store, "r");

    int current_tlb_index = 0;
    uint8_t insert_frame = 0;

    uint8_t page_number;
    uint8_t offset;

    uint16_t frame_number;

    int8_t value;

    tlb_entry *found_entry;

    l_list *c_node = &head;

    int i = 0;
    for(; i < num_of_addresses; i++)
    {
        page_number = (c_node->val & P_MASK) >> P_SHIFT;
        offset = (c_node->val & O_MASK);

        found_entry = search_tlb(page_number);

        if(found_entry != NULL)
        {
            tlb_hits++;

            frame_number = found_entry->f_number;
        } else
        {
            frame_number = page_tbl_fetch(page_number);

            if(frame_number == MISSING_PAGE)
            {
                page_faults++;

                fseek(bs_fp, page_number * PAGE_SIZE, SEEK_SET);
                fread(f_line, 1, PAGE_SIZE, bs_fp);

                phys_mem_insert(insert_frame, f_line);

                frame_number = insert_frame;
                insert_frame++;

                page_tbl_insert(page_number, frame_number);
            }

            tlb_insert(current_tlb_index % TLB_SIZE, page_number, frame_number);
            current_tlb_index++;
        }

        value = phys_mem_fetch(frame_number, offset);

        printf("Virtual address: %d Physical address: %d Value: %d\n", c_node->val, frame_number * PAGE_SIZE + offset, value);

        c_node = c_node->next;
    }

    fclose(bs_fp);

    printf("Number of Translated Addresses = %d\n", num_of_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", ((float) page_faults / (float) num_of_addresses));
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", ((float) tlb_hits / (float) num_of_addresses));

    free_tlb();
    free_l_list(&head);

    return 0;
}

// reads in address store file, populates linked list with addresses and returns number of addresses
int read_address_store(char *address_store, l_list *head)
{
    char f_line[MAX_BUFF];
    FILE *fp;

    fp = fopen(address_store, "r");

    if(fp == NULL)
    {
        fprintf(stderr, "error opening file %s\n", address_store);
        return -1;
    }

    l_list *c_node = head;

    int size = 0;
    while(fgets(f_line, MAX_BUFF, fp) != NULL)
    {
        c_node->val = atoi(f_line) & L_MASK;

        c_node->next = (l_list *) malloc(sizeof(l_list));

        c_node = c_node->next;

        size++;
    }

    free(c_node);

    fclose(fp);

    return size;
}

void initialize_tlb()
{
    int i = 0;
    for(; i < TLB_SIZE; i++)
    {
        tlb[i] = NULL;
    }
}

tlb_entry *search_tlb(uint8_t p_number)
{
    int i = 0;
    for(; i < TLB_SIZE; i++)
    {
        if(tlb[i] != NULL && tlb[i]->p_number == p_number)
        {
            return tlb[i];
        }
    }

    return NULL;
}

void tlb_insert(int index, uint8_t p_number, uint8_t f_number)
{
    tlb_entry *n_entry = (tlb_entry *) malloc(sizeof(tlb_entry));

    n_entry->p_number = p_number;
    n_entry->f_number = f_number;

    tlb[index] = n_entry;
}

void initialize_page_tbl()
{
    int i = 0;
    for(; i < NUM_PAGES; i++)
    {
        page_tbl[i] = MISSING_PAGE;
    }
}

uint16_t page_tbl_fetch(uint8_t p_number)
{
    return page_tbl[p_number];
}

void page_tbl_insert(int index, uint8_t f_number)
{
    page_tbl[index] = f_number;
}

char phys_mem_fetch(uint8_t f_number, uint8_t offset)
{
    return phys_mem[(f_number * PAGE_SIZE) + offset];
}

void phys_mem_insert(int f_number, char *page)
{
    int i = 0;
    for(; i < PAGE_SIZE; i++)
    {
        phys_mem[f_number * PAGE_SIZE + i] = page[i];
    }
}

void free_l_list(l_list *head)
{
    l_list *c_node = head;
    l_list *n_node = head;

    while(c_node != NULL)
    {
        n_node = c_node->next;
        free(c_node);
        c_node = n_node;
    }
}

void free_tlb()
{
    int i = 0;
    for(; i < TLB_SIZE; i++)
    {
        free(tlb[i]);
    }
}
