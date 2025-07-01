#include <stdio.h>
#include "inc/malloc.h"
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define TINY_ALLOC_SIZE 64
#define SMALL_ALLOC_SIZE 512
#define LARGE_ALLOC_SIZE 2048

static void write_str(const char *str)
{
    write(STDOUT_FILENO, str, strlen(str));
}


static void write_num(int num)
{
    char buffer[16];
    int len = 0;

    if (num == 0)
    {
        write(STDOUT_FILENO, "0", 1);
        return;
    }

    if (num < 0)
    {
        write(STDOUT_FILENO, "-", 1);
        num = -num;
    }

    while (num > 0)
    {
        buffer[len++] = '0' + (num % 10);
        num /= 10;
    }

    for (int i = len - 1; i >= 0; i--) {
        write(STDOUT_FILENO, &buffer[i], 1);
    }
}

static pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

static void write_thread_msg(const char *prefix, int thread_id, const char *suffix)
{
    pthread_mutex_lock(&output_mutex);
    write_str(prefix);
    write_num(thread_id);
    write_str(suffix);
    pthread_mutex_unlock(&output_mutex);
}


static volatile int total_allocations = 0;
static volatile int total_failures = 0;

void *thread_routine_with_helpers(void *arg)
{
    int thread_id = * (int *)arg;
    void *ptrs[50];
    int i;
    int local_failures = 0;

    for (i = 0;i < 50; i++)
    {
        size_t size = (i % 3 == 0) ? TINY_ALLOC_SIZE : 
                    (i % 3 == 1) ? SMALL_ALLOC_SIZE :
                    LARGE_ALLOC_SIZE;

        ptrs[i] = malloc(size);

        if (!ptrs[i]) {
            local_failures++;
            
        }
    }
    
    for (i =0; i < 50; i++)
    {
        if (ptrs[i]) 
            free(ptrs[i]);
    }

    if (local_failures == 0) {
        write_thread_msg("Thread ", thread_id, " SUCCESS - all 50 allocations\n");
    } else {
        write_thread_msg("Thread ", thread_id, " completed with ");
        pthread_mutex_lock(&output_mutex);
        write_num(local_failures);
        write_str(" failures out of 50\n");
        pthread_mutex_unlock(&output_mutex);
    }

    return NULL;
    
}

void test_multithreaded(void) {
    
    pthread_t threads[5];
    int thread_ids[5];
    int i;

    for (i = 0; i < 5; i++)
    {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_routine_with_helpers, &thread_ids[i]) != 0)
        {
            write_str("Failed to create thread ");
            write_num(i);
            write_str("\n");
            return;
        }
    }

    for (i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    write_str("All threads completed. Final memory state: \n");
    

}


int main(void) {
    write_str("=== Testing malloc implementation===\n");

    test_multithreaded();

    write_str("=== Testing complete ===\n");
}
