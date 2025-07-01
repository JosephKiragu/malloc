/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Joseph Kiragu                              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05             by Joseph           #+#    #+#             */
/*   Created: 2025-05             by Joseph          ###   ########.adl       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/malloc.h"

/* global state variable */
t_malloc_state g_malloc_state = {NULL, NULL, NULL, PTHREAD_MUTEX_INITIALIZER};


/* initializing once flag */
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static int initialized = 0;


// #ifdef DEBUG
// /* safe debug printing that doesn't use malloc */
// static void debug_print_size(size_t size)
// {
//     char    buffer[64];
//     int     len=0;
//     size_t  temp = size;
//     int     digits = 0;

//     /* count digits */
//     if (temp == 0){
//         digits = 1;
//     } else {
//         while (temp > 0) {
//             temp /= 10;
//             digits++;
//         }
//     }

//     /* build string backwards */
//     const char prefix[] = "malloc(";
//     const char suffix[] = ") called\n";

//     /* copy prefix */
//     for (int i = 0; prefix[i]; i++) {
//         buffer[len++] = prefix[i];
//     }

//     /* convert number to string */
//     int num_start = len;
//     temp = size;
//     for (int i = digits - 1; i >= 0; i--) {
//         buffer[num_start + i] = '0' + (temp % 10);
//         temp /= 10;
//     }
//     len += digits;

//     /* copy suffix */
//     for (int i = 0; suffix[i]; i++) {
//         buffer[len++] = suffix[i];
//     }

//     /* write directly to stderr to avoid malloc allocation */
//     write(STDERR_FILENO, buffer, len);

// }
// #endif

/* initialization function */
static void init_malloc_state(void)
{
    pthread_mutex_init(&g_malloc_state.mutex, NULL);
    g_malloc_state.tiny_zones = NULL;
    g_malloc_state.small_zones = NULL;
    g_malloc_state.large_zones = NULL;
    initialized = 1;
}



/* Allocate large blocks directly */
void *allocate_large(size_t size)
{
    t_zone      *zone;
    t_block     *block;
    // t_footer    *footer;

    // create a new zone just large enough for this allocation
    zone = create_zone(LARGE, size);
    if (!zone)
        return NULL;

    // get the block from the zone
    block = zone->first;

    // mark block as allocated
    block->is_free = 0;
    zone->free_blocks--;

    // return pointer to user data
    return (PTR_FROM_BLOCK(block));
}


/* main malloc implementation */
void *malloc(size_t size)
{
    t_zone      *zone;
    t_block     *block;
    t_zone_type zone_type;
    // static int in_malloc= 0; // prevent recursive debug prints
    // t_footer    *footer;
    
    /* ensuring initialization */
    pthread_once(&init_once, init_malloc_state);


    if (size == 0)
        return NULL;

    /* align size and add metadata overhead */
    size = BLOCK_SIZE(ALIGN(size));

    // #ifdef DEBUG
    // /* only print if we're not already in a malloc call (prevents recursion) */
    // if (__sync_bool_compare_and_swap(&in_malloc, 0, 1)) {
    //     debug_print_size(size);
    //     __sync_bool_compare_and_swap(&in_malloc, 1, 0);
    // }
    // #endif

    /* determine zone based on size */
    if (size <= BLOCK_SIZE(TINY_MAX))
        zone_type = TINY;
    else if (size <= BLOCK_SIZE(SMALL_MAX))
        zone_type = SMALL;
    else
    {
        pthread_mutex_lock(&g_malloc_state.mutex);
        void *result = allocate_large(size);
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return result;
    }

    /* lock for thread safety */
    pthread_mutex_lock(&g_malloc_state.mutex);

    /* try find a zone with enough space */
    zone = find_zone_with_space(size, zone_type);

    /* if no suitable zone found, create a new one */
    if (!zone)
    {
        zone = create_zone(zone_type, size);
        if (!zone)
        {
            pthread_mutex_unlock(&g_malloc_state.mutex);
            return NULL;
        }
    }

    /* find a free block in the zone */
    block = find_free_block(zone, size);

    /* split the block if needed */
    block = split_block(block, size);

    /* mark block as allocated */
    block->is_free = 0;
    zone->free_blocks--;

    /* unlock */
    pthread_mutex_unlock(&g_malloc_state.mutex);

    /* return pointer to user data area */
    return (PTR_FROM_BLOCK(block));
}

