/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Joseph Kiragu                              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05-02 10:00:00 by Joseph           #+#    #+#             */
/*   Updated: 2025-05-02 10:00:00 by Joseph          ###   ########.adl       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/malloc.h"

/* global state variable */
t_malloc_state g_malloc_state = {NULL, NULL, NULL, PTHREAD_MUTEX_INITIALIZER};


/* Allocate large blocks directly */
void *allocate_large(size_t size)
{
    t_zone      *zone;
    t_block     *block;
    t_footer    *footer;

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
    t_footer    *footer;

    /* handle edge case */
    if (size == 0)
        return NULL;

    /* align size and add metadata overhead */
    size = BLOCK_SIZE(ALIGN(size));

    /* determine zone based on size */
    if (size <= BLOCK_SIZE(TINY_MAX))
        zone_type = TINY;
    else if (size <= BLOCK_SIZE(SMALL_MAX))
        zone_type = SMALL;
    else
        return (allocate_large(size));

    /* lock for thread safety */
    pthread_mutex_lock(&gmalloc_state.mutex);

    /* try find a zone with enough space */
    zone = find_zone_with_space(size, zone_type);

    /* if no suitable zone found, create a new one */
    if (!zone)
    {
        zone = create_zone(zone_type, size);
        if (!zone)
        {
            pthread_mutex_unlock(g_malloc_state.mutex);
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
    pthread_mutex_unlock(&gmalloc_state.mutex);

    /* return pointer to user data area */
    return (PTR_FROM_BLOCK(block));
}

