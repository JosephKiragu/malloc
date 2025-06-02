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


int main(void)
{
    printf("==== TESTING MALLOC IMPLEMENTATION ===\n");

    test_basic_malloc_free();

    printf("\n=== tests completed ===\n");
    return (0);
}
