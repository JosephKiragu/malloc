/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Joseph <student@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05                                 #+#    #+#             */
/*   Updated: 2025-05                                ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef MALLOC_H
# define MALLOC_H

# include <stdlib.h>
# include <unistd.h>
# include <sys/mman.h>
# include <pthread.h>
# include <stdbool.h>
# include <stdint.h>
# include <stdio.h>

/*
 * Memory allocation size categories:
 * TINY: 1-128 bytes
 * SMALL: 129-1024 bytes
 * LARGE: 1025+ bytes
 */
# define TINY_MAX 128
# define SMALL_MAX 1024

/*
 * Zone sizes (in bytes)
 * TINY ZONE: 4 pages, approximately 16 KB
 * SMALL_ZONE: 32 pages, approximately 128 KB
 * LARGE_ZONE: sized to fit the allocation + metadata
 */

# define TINY_ZONE (getpagesize() * 4)
# define SMALL_ZONE (getpagesize() * 32)

/*
 * Alignment for memory allocations (16 bytes for SSE operations)
 */
# define ALIGNMENT 16
# define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// Zone types
typedef enum e_zone_type {
    TINY = 0,
    SMALL = 1,
    LARGE = 2
} t_zone_type;


//Block header - precedes each memory block
typedef struct s_block {
    size_t size;    // size includes the header and footer
    unsigned int is_free:1; // status flag
    struct s_block *next; // next block in free list (only maintained for free blocks)
} t_block;


// footer - at the end of each block
typedef size_t t_footer;


// zone header - managing a memory zone
typedef struct s_zone {
    size_t zone_size;       // total size of zone
    t_zone_type zone_type;  // zone type: TINY,SMALL, or Large
    size_t free_blocks;     // count of free blocks
    struct s_zone *next;    // next zone of same type
    t_block *first;         // first block in zone
} t_zone;


// global state
typedef struct s_malloc_state {
    t_zone *tiny_zones;     // list of TINY zones
    t_zone *small_zones;    // list of SMALL zones
    t_zone *large_zones;    // list of LARGE zones
    pthread_mutex_t mutex;  // for thread safety
} t_malloc_state;


// calculate footer position from a block header
# define FOOTER(block) ((t_footer *)((char *)(block)+(block)->size - sizeof(t_footer)))


// calculate block header position from a pointer returned to a user
# define BLOCK_FROM_PTR(ptr) ((t_block *)((char *)(ptr) - sizeof(t_block)))

// calculate user data pointer from a block header
# define PTR_FROM_BLOCK(block) ((void *)((char *)(block) + sizeof(t_block)))

// calculate size of block including all metadata
# define BLOCK_SIZE(size) (ALIGN(sizeof(t_block) + (size) + sizeof(t_footer)))

// function declarations
void    *malloc(size_t size);
void    free(void *ptr);
void    *realloc(void *ptr, size_t size);
void    show_alloc_mem(void);

/* internal helper functions */
size_t get_user_size(t_block *block);
t_zone  *create_zone(t_zone_type zone_type, size_t size);
t_zone  *find_zone_with_space(size_t size, t_zone_type zone_type);
t_block *find_free_block(t_zone *zone, size_t size);
t_block *split_block(t_block *block, size_t size);
void    merge_free_blocks(t_zone *zone, t_block *block);
void    *allocate_large(size_t size);
bool    try_extend_block(t_block *block, size_t new_size);
size_t    print_zone(t_zone *zone, t_zone_type zone_type);

#endif 
