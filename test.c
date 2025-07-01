#include <stdio.h>
#include "inc/malloc.h"

int main(void)
{
    printf("=== MINIMAL TEST: trying to isolate the 1024 small alloc ===\n");

    printf("Initial state (should be empty):\n");
    show_alloc_mem();

    printf("Allocating exactly 512 bytes:\n");
    void *ptr = malloc(512);
    printf("Allocated ptr: %p\n", ptr);

    printf("\nState after single 512-byte allocation:\n");
    show_alloc_mem();

    printf("\nFreeing the allocation");
    free(ptr);

    printf("\nState after free (should be empty again)\n");
    show_alloc_mem();

    return 0;
}
