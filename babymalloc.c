#include "babymalloc.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#define WSIZE 8
#define FULL_BLOCK_MIN_SIZE ((2 * WSIZE) + 8) // 2 words for header and footer, 8 bytes for payload
#define ALIGN(size) (((size) + (WSIZE - 1)) & ~0x7)
#define GET_SIZE(p) (*(uint64_t *) (p) & ~0x7)
#define GET_USED(p) (*(uint64_t *) (p) & 0x1) // 0 for free, 1 for used
#define SET_USED(p) (*(uint64_t *) (p) |= 0x1)
#define SET_FREE(p) (*(uint64_t *) (p) &= ~0x1)
#define GET_NEXT_BLKP(p) ((char *) (p) + GET_SIZE(p) + 2 * WSIZE)
#define GET_BLK_FOOTERP(p) ((char *) (p) + GET_SIZE(p) + WSIZE)
#define GET_PREV_BLKP(p) ((char *) (p) - (GET_SIZE((p) - WSIZE) + 2 * WSIZE))
#define GET_PAYLOAD(p) ((char *) (p) + WSIZE)

void *heap_startp;
void *heap_endp;

static void *extend_heap(size_t size);
static void *find_fit(size_t size);
static void insert_block(void *blkp, size_t size);
static void coalesce(void *blkp);

void *babymalloc(size_t size) {
    size_t padding = -size & (WSIZE - 1);
    size_t aligned_size = ALIGN(size);

    // check for exceeding the maximum size or integer overflow
    if (aligned_size > INTPTR_MAX - 2 * WSIZE || size > SIZE_MAX - padding) {
        return NULL;
    }

    if (!heap_startp) {
        heap_startp = extend_heap(aligned_size);
        if (!heap_startp) {
            return NULL;
        }

        heap_endp = GET_NEXT_BLKP(heap_startp);
    }

    void *blkp = find_fit(aligned_size);
    if (blkp) {
        insert_block(blkp, aligned_size);
        return GET_PAYLOAD(blkp);
    }

    blkp = extend_heap(aligned_size);
    if (!blkp) {
        return NULL;
    }

    heap_endp = GET_NEXT_BLKP(blkp);
    insert_block(blkp, aligned_size);
    return GET_PAYLOAD(blkp);
}

void babyfree(void *ptr) {
    void *blkp = (char *) ptr - WSIZE;
    SET_FREE(blkp);
    SET_FREE(GET_BLK_FOOTERP(blkp));
    memset(ptr, 0, GET_SIZE(blkp));
    coalesce(blkp);
}

void new_heap() {
    size_t heap_size = (char *) heap_endp - (char *) heap_startp;
    memset(heap_startp, 0, heap_size);
    heap_startp = NULL;
    heap_endp = NULL;
}

static void *extend_heap(size_t size) {
    void *blkp = sbrk((intptr_t) size + 2 * WSIZE);  // 2 words for header and footer
    if (blkp == (void *) -1) {
        fprintf(stderr, "sbrk failed to allocate %zu: %s (errno %d)\n", size, strerror(errno), errno);
        return NULL;
    }

    *(uint64_t *) blkp = size;
    *(uint64_t *) GET_BLK_FOOTERP(blkp) = size;
    SET_FREE(blkp);
    SET_FREE(GET_BLK_FOOTERP(blkp));
    return blkp;
}

// first-fit algorithm
static void *find_fit(size_t size) {
    void *blkp = heap_startp;

    while (*(uint64_t *) blkp) {
        if (!GET_USED(blkp) && *(uint64_t *) blkp >= size) {
            return blkp;
        }
        blkp = GET_NEXT_BLKP(blkp);
    }
    return NULL;
}

static void insert_block(void *blkp, size_t size) {
    if (GET_SIZE(blkp) - size >= FULL_BLOCK_MIN_SIZE) { // split the block if the remaining space is enough
        size_t new_blk_size = GET_SIZE(blkp) - size - 2 * WSIZE;

        *(uint64_t *) blkp = size;
        *(uint64_t *) GET_BLK_FOOTERP(blkp) = size;

        void *new_blkp = GET_NEXT_BLKP(blkp);
        *(uint64_t *) new_blkp = new_blk_size;
        *(uint64_t *) GET_BLK_FOOTERP(new_blkp) = new_blk_size;
        SET_FREE(new_blkp);
        SET_FREE(GET_BLK_FOOTERP(new_blkp));
    }

    SET_USED(blkp);
    SET_USED(GET_BLK_FOOTERP(blkp));
}

static void coalesce(void *blkp) {
    size_t blk_size = GET_SIZE(blkp);
    void *prev_blkp = NULL;
    void *next_blkp = GET_NEXT_BLKP(blkp);

    int coalesce_prev = 0;
    int coalesce_next = 0;

    // check if the block is the first block
    // if it is, we don't need to coalesce with the previous block
    if (blkp == heap_startp) {
        coalesce_prev = 0;
    } else {
        prev_blkp = GET_PREV_BLKP(blkp);
        if (!GET_USED(prev_blkp)) {
            coalesce_prev = 1;
        }
    }

    // check if the block is the last block
    // if it is, we don't need to coalesce with the next block
    if (next_blkp >= heap_endp) {
        coalesce_next = 0;
    } else if (!GET_USED(next_blkp)) {
        coalesce_next = 1;
    }

    if (coalesce_prev && coalesce_next) {
        blk_size += GET_SIZE(prev_blkp) + GET_SIZE(next_blkp) + 4 * WSIZE;
        *(uint64_t *) prev_blkp = blk_size;
        *(uint64_t *) GET_BLK_FOOTERP(next_blkp) = blk_size;
        SET_FREE(prev_blkp);
        SET_FREE(GET_BLK_FOOTERP(next_blkp));
    } else if (coalesce_prev) {
        blk_size += GET_SIZE(prev_blkp) + 2 * WSIZE;
        *(uint64_t *) prev_blkp = blk_size;
        *(uint64_t *) GET_BLK_FOOTERP(blkp) = blk_size;
        SET_FREE(prev_blkp);
        SET_FREE(GET_BLK_FOOTERP(blkp));
    } else if (coalesce_next) {
        blk_size += GET_SIZE(next_blkp) + 2 * WSIZE;
        *(uint64_t *) blkp = blk_size;
        *(uint64_t *) GET_BLK_FOOTERP(next_blkp) = blk_size;
        SET_FREE(blkp);
        SET_FREE(GET_BLK_FOOTERP(next_blkp));
    }

    if (prev_blkp == heap_startp) {
        *(uint64_t *) heap_startp = blk_size;
    }
}

void print_heap() {
    void *blkp = heap_startp;
    while (*(uint64_t *) blkp) {
        if (GET_USED(blkp)) {
            // use PRIu64 to print uint64_t
            printf("Block at %p: %" PRIu64 " size %" PRIu64 ", used\n", blkp, GET_SIZE(blkp), GET_SIZE(GET_BLK_FOOTERP(blkp)));
        } else {
            printf("Block at %p: %" PRIu64 " size %" PRIu64 ", free\n", blkp, GET_SIZE(blkp), GET_SIZE(GET_BLK_FOOTERP(blkp)));
        }
        blkp = GET_NEXT_BLKP(blkp);
    }
}
