/*
    Copyright (c) 2012 250bpm s.r.o.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "alloc.h"

#if defined SP_ALLOC_MONITOR

#include "mutex.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct sp_alloc_hdr {
    size_t size;
    const char *name;
};

static struct sp_mutex sp_alloc_sync;
static size_t sp_alloc_bytes;
static size_t sp_alloc_blocks;

void sp_alloc_init (void)
{
    sp_mutex_init (&sp_alloc_sync);
    sp_alloc_bytes = 0;
    sp_alloc_blocks = 0;
}

void sp_alloc_term (void)
{
    sp_mutex_term (&sp_alloc_sync);
}

void *sp_alloc_ (size_t size, const char *name)
{
    uint8_t *chunk;

    chunk = malloc (sizeof (struct sp_alloc_hdr) + size);
    if (!chunk)
        return NULL;

    sp_mutex_lock (&sp_alloc_sync);
    ((struct sp_alloc_hdr*) chunk)->size = size;
    ((struct sp_alloc_hdr*) chunk)->name = name;
    sp_alloc_bytes += size;
    ++sp_alloc_blocks;
    printf ("Allocating %s (%zu bytes)\n", name, size);
    printf ("Current memory usage: %zu bytes in %zu blocks\n",
        sp_alloc_bytes, sp_alloc_blocks);
    sp_mutex_unlock (&sp_alloc_sync);

    return chunk + sizeof (struct sp_alloc_hdr);
}

void *sp_realloc (void *ptr, size_t size)
{
    struct sp_alloc_hdr *oldchunk;
    struct sp_alloc_hdr *newchunk;
    size_t oldsize;

    oldchunk = ((struct sp_alloc_hdr*) ptr) - 1;
    oldsize = oldchunk->size;
    newchunk = realloc (oldchunk, sizeof (struct sp_alloc_hdr) + size);
    if (!newchunk)
        return NULL;
    newchunk->size = size;

    sp_mutex_lock (&sp_alloc_sync);
    sp_alloc_bytes -= oldsize;
    sp_alloc_bytes += size;
    printf ("Reallocating %s (%zu bytes to %zu bytes)\n",
        newchunk->name, oldsize, size);
    printf ("Current memory usage: %zu bytes in %zu blocks\n",
        sp_alloc_bytes, sp_alloc_blocks);
    sp_mutex_unlock (&sp_alloc_sync);

    return newchunk + sizeof (size_t);
}

void sp_free (void *ptr)
{
    struct sp_alloc_hdr *chunk;
    
    if (!ptr)
        return;
    chunk = ((struct sp_alloc_hdr*) ptr) - 1;

    sp_mutex_lock (&sp_alloc_sync);
    sp_alloc_bytes -= chunk->size;
    --sp_alloc_blocks;
    printf ("Deallocating %s (%zu bytes)\n", chunk->name, chunk->size);
    printf ("Current memory usage: %zu bytes in %zu blocks\n",
        sp_alloc_bytes, sp_alloc_blocks);
    sp_mutex_unlock (&sp_alloc_sync);

    free (chunk);
}

#else

#include <stdlib.h>

void sp_alloc_init (void)
{
}

void sp_alloc_term (void)
{
}

void *sp_alloc_ (size_t size)
{
    return malloc (size);
}

void *sp_realloc (void *ptr, size_t size)
{
    return realloc (ptr, size);
}

void sp_free (void *ptr)
{
    free (ptr);
}

#endif

