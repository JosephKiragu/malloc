/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   show_alloc.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Joseph <student@42.Adl>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05             by Joseph           #+#    #+#             */
/*   Updated: 2025-05             by Joseph          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/malloc.h"

extern t_malloc_state g_malloc_state;

/*
 * printing a zone and its allocations
 */

void print_zone(t_zone *zone, t_zone_type zone_type)
{
    t_block *block;
    size_t  total_bytes;

    // print zone types and addresses
    if (zone_type == TINY)
        printf("TINY : %p\n", (void *)zone);
    else if (zone_type == SMALL)
        printf("SMALL : %p\n", (void *)zone);
    else
        printf("LARGE : %p\n", (void *)zone);

    /* iterate through blocks in the zone */
    block = zone->first;
    total_bytes = 0;

    while (block)
    {
        /* only print allocated blocks */
        if (!block->is_free)
        {
            void *start_addr = PTR_FROM_BLOCK(block);
            void *end_addr = (void *)((char *)start_addr + get_user_size(block) - 1);
            size_t block_size = get_user_size(block);

            printf("%p - %p : %zu bytes\n", start_addr, end_addr, block_size);
            total_bytes += block_size;
        }

        /* move to the next physical block */
        if ((char *)block + block->size < (char *)zone + zone->zone_size)
            block = (t_block *)((char *)block + block->size);
        else
            break;

    }

    return (total_bytes);
}


/*
 *  show memory allocation information
 */
void show_alloc_mem(void)
{
    t_zone  *zone;
    size_t  total_bytes;

    /* locking for thread safety */
    pthread_mutex_lock(&g_malloc_state.mutex);

    total_bytes = 0;

    /* print tiny zones */
    zone = g_malloc_state.tiny_zones;
    while (zone)
    {
        total_bytes += print_zone(zone, TINY);
        zone = zone->next;
    }

    /* print SMALL zones */
    zone = g_malloc_state.small_zones;
    while (zone)
    {
        total_bytes += print_zone(zone, SMALL);
        zone = zone->next;
    }

    /* print LARGE zones */
    zone= g_malloc_state.large_zones;
    while (zone) 
    {
        total_bytes += print_zone(zone, LARGE);
        zone = zone->next;
    }

    /* print total allocated zones */
    printf("Total: %zu \n", total_bytes);

    /* unlock mutex */
    pthread_mutex_unlock(&g_malloc_state.mutex);

}
