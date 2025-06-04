#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "inc/malloc.h"

#define TINY_ALLOC_SIZE 64
#define SMALL_ALLOC_SIZE 512
#define LARGE_ALLOC_SIZE 2048
#define NUM_ALLOCS 200


void test_basic_malloc_free(void)
{
    printf("\n=== testing basic malloc and free ===\n");

    /* testing tiny malloc */
    char *ptr1 = (char *)malloc(TINY_ALLOC_SIZE);
    if (ptr1)
    {
        strcpy(ptr1, "This is a tiny allocation");
        printf("Tiny allocation at %p: %s\n", ptr1, ptr1);
        free(ptr1);
        printf("Freed tiny allocation\n");
    }
    else
        printf("Failed to allocate tiny memory\n");

    /* small allocation*/
    char *ptr2 = (char *)malloc(SMALL_ALLOC_SIZE);
    if (ptr2)
    {
        strcpy(ptr2,"This is a small allocation");
        printf("Small allocation at %p: %s\n", ptr2, ptr2);
        free(ptr2);
        printf("Freed small allocation\n");
    }
    else
        printf("Failed to allocate small allocation\n");

    /* large allocation */
    char *ptr3 = (char *)malloc(LARGE_ALLOC_SIZE);
    if (ptr3)
    {
        strcpy(ptr3, "This is a large allocation");
        printf("Large allocation at %p: %s\n", ptr3, ptr3);
        free(ptr3);
        printf("Freed large allocation\n");
    }
    else
        printf("Failed to allocate small allocation\n");
}

void test_realloc(void)
{
    printf("\n=== Testing realloc ===\n");

    char *ptr = (char *)malloc(TINY_ALLOC_SIZE);
    if (ptr)
    {
        strcpy(ptr, "initial small string");
        printf("Initial allocation at %p: %s\n", ptr, ptr);

        /* growing the allocation */
        ptr = (char *)realloc(ptr, SMALL_ALLOC_SIZE);
        if (ptr)
        {
            strcpy(ptr + strlen(ptr), " - now expanded to a much longer string that wouldn't fit before");
            printf("Expanded allocation at %p: %s\n", ptr, ptr);

            /* shrinking the allocation */
            ptr= (char *)realloc(ptr, TINY_ALLOC_SIZE);
            if (ptr)
            {
                printf("shrunk allocation at %p: %s\n", ptr, ptr);
                free(ptr);
                printf("freed realloc'd memory\n");
            }
            else
                printf("failed to shrink allocation\n");
        }
        else
            printf("failed to expand allocation\n");
    }
    else
        printf("failed initial allocation for reallloc test\n");
}

void test_multiple_allocations(void)
{
    printf("\n=== Testing multiple allocations ===\n");

    void *ptrs[NUM_ALLOCS];
    int i;

    /* allocate many blocks of different sizes */
    for (i=0; i<NUM_ALLOCS; i++)
    {
        size_t size;

        if (i % 3 == 0)
            size = TINY_ALLOC_SIZE;
        else if (i % 3 == 1)
            size = SMALL_ALLOC_SIZE;
        else
            size = LARGE_ALLOC_SIZE;

        ptrs[i] = malloc(size);

        if (!ptrs[i])
        {
            printf("failed to allocate at iteration %d\n", i);
            break;
        }
    }

    int tiny_counts = 0, small_counts = 0, large_counts = 0;
    for (int i = 0; i < NUM_ALLOCS; i++)
        {if (i % 3 == 0) tiny_counts++;
        else if (i % 3 == 1) small_counts++;
        else large_counts++;
    }

    printf("TINY: %d, SMALL: %d, LARGE: %d\n", tiny_counts, small_counts, large_counts);

    /* show memory state */
    printf("Memory state after allocations:\n");
    show_alloc_mem();

    /* free half the allocations */
    for (i = 0; i < NUM_ALLOCS; i += 2)
    {
        if (ptrs[i])
            free(ptrs[i]);
    }

    printf("Memory state after freeing half:\n");
    show_alloc_mem();

    /* free the test */
    for (i=1; i < NUM_ALLOCS; i += 2)
    {
        if (ptrs[i])
            free(ptrs[i]);
    }

    printf("Memory state after freeing all\n");
    show_alloc_mem();
}


int main(void)
{
    printf("==== TESTING MALLOC IMPLEMENTATION ===\n");

    // test_basic_malloc_free();
    // test_realloc();
    test_multiple_allocations();

    printf("\n=== tests completed ===\n");
    return (0);
}
