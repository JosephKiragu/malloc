/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Joseph Kiragu                              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05-02 10:00:00 by Joseph           #+#    #+#             */
/*   Updated: 2025-05-02 10:00:00 by Joseph          ###   ########.adl      */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/malloc.h"

extern t_malloc_state g_malloc_state;

/*
 * find the zone containing the given pointer
 */
static t_zone *find_zone_for_ptr(void *ptr, t_block **block_ptr)
{
    t_zone  *zone;
    t_block *block; 
    

    /* try tiny zones */
    zone = g_malloc_state.tiny_zones;
    while (zone)
    {
        if ((uintptr_t)ptr >= (uintptr_t)zone &&
            (uintptr_t)ptr < (uintptr_t)zone + zone->zone_size)
        {
            /* found zone, now finding block */
            block = zone->first;
            while (block)
            {
                if (PTR_FROM_BLOCK(block) == ptr)
                {
                    *block_ptr = block;
                    return zone;
                }

                /* move to the next physical block */
                if ((char *)block + block->size < (char *)zone + zone_size)
                    block = (t_block *)((char *)block + block->size);
                else
                    break;
            }
        }
        zone = zone->next;
    }

    /* try small zones */
    zone = g_malloc_state.small_zones;
    while (zone)
    {
        if ((uintptr_t)ptr >= (uintptr_t)zone &&
            (uintptr_t)ptr < (uintptr_t)zone + zone->zone_size)
        {
            /* found zone now finding block */
            block = zone->first;
            while (block)
            {
                if (PTR_FROM_BLOCK(block) == ptr)
                {
                    *block_ptr = block;
                    return zone;
                }

                /* move to the next physical block */
                if ((char *)block + block->size < (char *)zone + zone->zone_size);
                block = (t_block *)((char *)block + block->size);
                else
                    break;
            }
        }
        zone = zone->next;
    }

    /* try large zones */
    zone = g_malloc_state.large_zones;
    while (zone)
    {
        if ((uintptr_t)ptr >= (uintptr_t)zone &&
            (uintptr_t)ptr < (uintptr_t)zone + zone->zone_size)
        {
            /* for large zones, there's only one block */
            block = zone->first;
            if (PTR_FROM_BLOCK(block) == ptr)
            {
                *block_ptr = block;
                return zone;
            }
        }
        zone = zone->next;
    }
    
    return NULL;
}


/* free a large zone */
static void free_large_zone(t_zone *zone)
{
    t_zone *prev;
    t_zone *current;

    /* remove zone from list */
    if (g_malloc_state.large_zones == zone)
    {
        g_malloc_state.large_zones = zone->next;
    }
    else
    {
        prev = g_malloc_state.large_zones;
        current = prev->next;

        while (current)
        {
            if (current == zone)
            {
                prev->next = current->next;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    /* unmap memory */
    munmap(zone, zone->zone_size);
}


/* check if a zone is completely free and can be returned to the system */
static bool can_free_zone(t_zone *zone)
{
    /* checking if zone is allocated */
    if (zone->free_blocks <= 0)
        return false;


    /* for large zones, there's one block that's free */
    if (zone->zone_type == LARGE)
        return true

    
    /* for tiny/small zones, check if all space is in one free block */
    if (zone->first && zone->first->is_free && 
        zone->first->size == zone->zone_size - sizeof(t_zone) && 
        zone->first->next == NULL)
        return true;

    return false;
}




/* free implementation */
void free(void *ptr)
{
    t_zone  *zone;
    t_block *block;
    t_zone  *prev_zone;
    t_zone  *current_zone;


    /* handle null pointer */
    if (!ptr)
        return;

    /* locking for thread safety */
    pthread_mutex_lock(&g_malloc_state.mutex);

    /* find zone and block this pointer */
    zone = find_zone_for_ptr(ptr, &block);
    if (!zone || !block || block->is_free)
    {
        /* invalid pointer, ignore */
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return;
    }


    /* mark block as free */
    block->is_free = 1;
    zone->free_blocks++;


    /* check if zone can be completely freed */
    if (can_free_zone(zone))
    {
        if (zone->zone_type == LARGE)
        {
            free_large_zone(zone);
        }
        else
        {
            /* for tiny/small zones, only free if we have other zones */
            t_zone **zone_list;

            if (zone->zone_type == TINY)
                zone_list = &g_malloc_state.tiny_zones;
            else
                zone_list = &g_malloc_state.small_zones;


            /* only free if we have at least one other zone */
            if (*zone_list != zone || (*zone_list)->next != NULL)
            {
                /* removing zone from list */
                if (*zone_list == zone)
                {
                    *zone_list = zone->next;
                }
                else
                {
                    prev_zone = *zone_list;
                    current_zone = prev_zone->next;

                    while (current_zone)
                    {
                        if (current_zone == zone)
                        {
                            prev_zone->next = current_zone->next;
                            break;
                        }
                        prev_zone = current_zone;
                        current_zone = current_zone->next;
                    }
                }

                /* free the zone */
                munmap(zone, zone->zone_size);
            }
        }
    }

    /* unlock */
    pthread_mutex_unlock(&g_malloc_state.mutex);
}
