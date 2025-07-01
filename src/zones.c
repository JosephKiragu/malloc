/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   zones.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Joseph Kiragu                              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05           by Kiragu             #+#    #+#             */
/*   Updated: 2025-05           by Kiragu           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/malloc.h"

extern t_malloc_state g_malloc_state;


/*
 * Get the actual size of data a block can hold
 */
size_t get_user_size(t_block *block)
{
    return (block->size - sizeof(t_block) - sizeof(t_footer));
}

/* create a new zone of the specified type with at least the given size */
t_zone *create_zone(t_zone_type zone_type, size_t size)
{
    t_zone  *zone;
    size_t  zone_size;
    t_block *block;
    t_footer *footer;

    /* determine zone size based on type */
    if (zone_type == TINY)
        zone_size = TINY_ZONE;
    else if (zone_type == SMALL)
        zone_size = SMALL_ZONE;
    else // large
        zone_size = ALIGN(sizeof(t_zone) + BLOCK_SIZE(size));

    /* map memory for the zone */
    zone = mmap(NULL, zone_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (zone == MAP_FAILED)
        return NULL;

    /* initialize zone header */
    zone->zone_size = zone_size;
    zone->zone_type = zone_type;
    zone->free_blocks = 1;
    zone->next = NULL;

    /* initialize the first (and only) block in the zone */
    block = (t_block *)((char *)zone + sizeof(t_zone));
    block->size = zone_size - sizeof(t_zone);
    block->is_free = 1;
    block->next = NULL;

    /* set up footer */
    footer = FOOTER(block);
    *footer = block->size;

    /* set first block pointer */
    zone->first = block;

    /* add to appropriate zone list based on type */
    if (zone_type == TINY)
    {
        zone->next = g_malloc_state.tiny_zones;
        g_malloc_state.tiny_zones = zone;
    }
    else if (zone_type == SMALL)
    {
        zone->next = g_malloc_state.small_zones;
        g_malloc_state.small_zones = zone;
    }
    else // large
    {
        zone->next = g_malloc_state.large_zones;
        g_malloc_state.large_zones = zone;
    }

    return zone;
}



/* 
 * find a zone with enough free space for the specified size
 */
t_zone *find_zone_with_space(size_t size, t_zone_type zone_type)
{
    t_zone      *zone;
    t_block     *block;

    // select zone list based on type
    if (zone_type == TINY)
        zone = g_malloc_state.tiny_zones;
    else if (zone_type == SMALL)
        zone = g_malloc_state.small_zones;
    else
        zone = g_malloc_state.large_zones;

    // seacrching through the zones of the specified type
    while (zone)
    {
        // check if this zone has a suitable free block
        block = find_free_block(zone, size);
        if (block)
            return zone;

        zone = zone->next;
    }

    // no suitable zones found
    return NULL;
}


/*
 * find a free block in the specified zone that's large enough
 */
t_block *find_free_block(t_zone *zone, size_t size)
{
    t_block     *block;

    block = zone->first;
    while (block)
    {
        if (block->is_free && block->size >= size)
            return block;
        
        // get next block by advancing pointer
        if ((char *)block + block->size < (char *)zone + zone->zone_size)
            block = (t_block *)((char *)block + block->size);
        else
            break;
    }

    return NULL;
}


/*
 * split a block if its larger than needed
 */
t_block *split_block(t_block *block, size_t size)
{
    t_block     *new_block;
    t_footer    *footer;
    size_t      remaining_size;

    // check if block is large enough to split
    if (block->size < size + BLOCK_SIZE(1))
        return block;

    // calculate remaining size
    remaining_size = block->size - size;

    //update current block's size
    block->size = size;
    footer = FOOTER(block);
    *footer = size;

    // create a new block from remaining space
    new_block = (t_block *)((char *)block + size);
    new_block->size = remaining_size;
    new_block->is_free = 1;

    // set up footer for new block
    footer = FOOTER(new_block);
    *footer = remaining_size;

    // update next pointers
    new_block->next = block->next;
    block->next = new_block;

    return block;
}



/*
 * merge adjacent free block
 */
void merge_free_blocks(t_zone *zone, t_block *block)
{
    t_block     *next_block;
    t_footer    *footer;
    t_block     *prev_block;
    t_footer     *prev_footer;

    //check if there's a next block to potentially merge with
    if ((char *)block + block->size < (char *)zone + zone->zone_size)
    {
        next_block = (t_block *)((char *)block + block->size);
        if (next_block->is_free)
        {
            // merge with next block
            block->size += next_block->size;
            block->next = next_block->next;

            // update footer
            footer = FOOTER(block);
            *footer = block->size;

            // dcerease free block count as we merged two blocks
            zone->free_blocks--;
        }
    }

    // check if there's a previous block to potentially merge with
    if ((char *)block > (char *)zone->first)
    {
        // use boundary tag to find previous block's size
        prev_footer = (t_footer *)((char *)block - sizeof(t_footer));
        prev_block = (t_block *)((char *)block - *prev_footer);

        if (prev_block->is_free)
        {
            // merge with previous block
            prev_block->size += block->size;
            prev_block->next = block->next;

            // update footer
            footer = FOOTER(prev_block);
            *footer = prev_block->size;

            // decrease free block count as we merged two blocks
            zone->free_blocks--;
        }
    }
}





/*
 * check if a block can be extended
 */

bool try_extend_block(t_block *block, size_t new_size)
{
    t_block     *next_block;
    size_t      combined_size;
    t_footer    *footer;

    // check if there's a new block
    next_block = (t_block *)((char *)block + block->size);

    // if next block exists and is free
    if (next_block && next_block->is_free)
    {
        combined_size = block->size +next_block->size;

        // if combined size is enough
        if (combined_size >= new_size)
        {
            // merge with next block
            block->size = combined_size;
            block->next = next_block->next;

            // update footer
            footer = FOOTER(block);
            *footer = block->size;

            // if resulting block is much larger than needed, split it
            if (block->size > new_size + BLOCK_SIZE(1))
                split_block(block, new_size);


            return true;
        }
    }
    return false;
}




