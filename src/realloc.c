/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Joseph K                                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025-05-02 10:00:00 by Joseph          #+#    #+#             */
/*   Updated: 2025-05-02 10:00:00 by Joseph         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/malloc.h"


extern t_malloc_state g_malloc_state;

/* copy data from src to dst, up to size bytes */
static void ft_memcpy(void *dst, const void *src, size_t size)
{
    size_t      i;
    char        *d;
    const char  *s;

    d = (char *)dst;
    s = (const char *)src;
    i = 0;
    while (i < size)
    {
        d[i] = s[i];
        i++;
    }
}


/* get the actual size of data a block can hold */
static size_t get_user_size(t_block *block)
{
    return (block->size - sizeof(t_block) - sizeof(t_footer));
}


/* realloc implementation */
void *realloc(void *ptr, size_t size)
{
    void    *new_ptr;
    t_block *block;
    size_t  user_size;
    size_t  aligned_size;

    // handle edge cases
    if (!ptr)
        return (malloc(size));
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }


    pthread_mutex_lock(&g_malloc_state.mutex);


    /* getting block header from ptr */
    block = BLOCK_FROM_PTR(ptr);

    /* getting current user size */
    user_size = get_user_size(block);

    /* calculate required size with alignment */
    aligned_size = BLOCK_SIZE(ALIGN(size));

    /* if new size fits in current block, return same pointer */
    if (aligned_size <= block->size)
    {
        /* if the block is much larger than needed, split it */
        if (block->size >= aligned_size + BLOCK_SIZE(1))
            split_block(block, aligned_size);

        pthread_mutex_unlock(&g_malloc_state.mutex);
        return ptr;
    }

    /* try to extend the block if possible */
    if (try_extend_block(block, aligned_size))
    {
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return ptr;
    }

    /* unlock before calling malloc */
    pthread_mutex_unlock(&g_malloc_state.mutex);

    /* allocate new memory */
    new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    /* copy data to the new location */
    ft_memcpy(new_ptr, ptr, user_size, user_size < size ? user_size : size);


    /* free old memory */
    free(ptr);


    return (new_ptr);

}
