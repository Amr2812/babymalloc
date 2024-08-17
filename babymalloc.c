#include "babymalloc.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

// #define DEBUG
#define WSIZE 8
#define ALIGN(size) (((size) + (WSIZE - 1)) & ~0x7)
#define GET_SIZE(p) (*(uint64_t *) (p) & ~0x7)
#define GET_USED(p) (*(uint64_t *) (p) & 0x1) // 0 for free, 1 for used
#define SET_USED(p) (*(uint64_t *) (p) |= 0x1)
#define SET_FREE(p) (*(uint64_t *) (p) &= ~0x1)

#ifdef DEBUG
#define debug_print(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define debug_print(fmt, ...)
#endif

void *heap_startp;

static void *extend_heap(size_t size);
static void *find_fit(size_t size);
static void insert_block(void *blkp, size_t size);

void *babymalloc(size_t size) {
    size = ALIGN(size);
    if (!heap_startp) {
        heap_startp = extend_heap(size);
        debug_print("heap_startp: %p\n", heap_startp);
        debug_print("heap_startp value: %lu\n", *(uint64_t *) heap_startp);
        if (heap_startp == (void *) -1) {
            return NULL;
        }
    }

    void *blkp = find_fit(size);
    if (blkp) {
        insert_block(blkp, size);
        return blkp + WSIZE; // return the payload address (after the header)
    }

    blkp = extend_heap(size);
    if (!blkp) {
        return NULL;
    }

    insert_block(blkp, size);
    return blkp + WSIZE;
}

void babyfree(void *ptr) {
    void *blkp = ptr - WSIZE;
    SET_FREE(blkp);
    SET_FREE(blkp + GET_SIZE(blkp) + WSIZE);
    memset(ptr, 0, GET_SIZE(blkp));
}

static void *extend_heap(size_t size) {
    void *blkp = sbrk(size + 2 * WSIZE);  // 2 words for header and footer
    if (blkp == (void *) -1) {
        return NULL;
    }

    *(uint64_t *) blkp = size;
    SET_FREE(blkp);
    return blkp;
}

// first-fit algorithm
static void *find_fit(size_t size) {
    void *blkp = heap_startp;

    debug_print("blkp: %p\n", blkp);
    debug_print("blkp value: %lu\n", *(uint64_t *) blkp);
    while (*(uint64_t *) blkp) {
        if (!GET_USED(blkp) && *(uint64_t *) blkp >= size) {
            return blkp;
        }
        blkp += (GET_SIZE(blkp) + 2 * WSIZE);
    }
    return NULL;
}

static void insert_block(void *blkp, size_t size) {
    SET_USED(blkp); // set header
    SET_USED(blkp + size + WSIZE); // set footer
}

void print_heap() {
    void *blkp = heap_startp;
    while (*(uint64_t *) blkp) {
        if (GET_USED(blkp)) {
            printf("Block at %p: size %lu, used\n", blkp, GET_SIZE(blkp));
        } else {
            printf("Block at %p: size %lu, free\n", blkp, GET_SIZE(blkp));
        }
        blkp += (GET_SIZE(blkp) + 2 * WSIZE);
    }
}
