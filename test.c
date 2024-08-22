//
// Created by amr on 8/15/24.
//

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "babymalloc.h"

#define WSIZE sizeof(size_t)

void test_new_heap() {
    void *ptr = babymalloc(16);
    void *ptr2 = babymalloc(16);
    void *ptr3 = babymalloc(16);
    babyfree(ptr);
    babyfree(ptr2);
    babyfree(ptr3);
    new_heap();
    void *ptr4 = babymalloc(48);

    assert(ptr != ptr4);
}

void test_free_then_malloc_alignment() {
    void *ptr = babymalloc(WSIZE - 1);
    void *ptr2 = babymalloc(WSIZE - 1);
    babyfree(ptr);
    void *ptr3 = babymalloc(WSIZE);

    assert(ptr == ptr3);
}

void test_prev_block_coalescing() {
    void *ptr = babymalloc(WSIZE * 2);
    void *ptr2 = babymalloc(WSIZE * 2);
    babyfree(ptr);
    babyfree(ptr2);
    void *ptr3 = babymalloc(WSIZE * 4);

    assert(ptr == ptr3);
}

void test_next_block_coalescing() {
    void *ptr = babymalloc(WSIZE * 2); // 16 on 64-bit systems
    void *ptr2 = babymalloc(WSIZE * 2);
    babyfree(ptr2);
    babyfree(ptr);
    void *ptr3 = babymalloc(WSIZE * 4); // 32 on 64-bit systems

    assert(ptr == ptr3);
}

void test_next_and_prev_block_coalescing() {
    void *ptr = babymalloc(WSIZE * 2); // 16 on 64-bit systems
    void *ptr2 = babymalloc(WSIZE * 2);
    void *ptr3 = babymalloc(WSIZE * 2);
    babyfree(ptr);
    babyfree(ptr3);
    babyfree(ptr2);
    void *ptr4 = babymalloc(WSIZE * 6); // 48 on 64-bit systems

    assert(ptr == ptr4);
}

void test_block_splitting() {
    char *ptr = (char *) babymalloc(WSIZE * 4); // 32 on 64-bit systems
    babyfree(ptr);
    char *ptr2 = (char *) babymalloc(WSIZE); // 8 on 64-bit systems
    char *ptr3 = (char *) babymalloc(WSIZE);

    assert(ptr == ptr2);
    assert(ptr3 - ptr2 == WSIZE * 3); // 24 on 64-bit systems
}

void test_max_size() {
    void *ptr = babymalloc(INTPTR_MAX - 2 * WSIZE);
    void *ptr2 = babymalloc(SIZE_MAX); // test for overflow

    assert(ptr == NULL);
    assert(ptr2 == NULL);
}

int main(int argc, char **argv) {
    test_new_heap();

    new_heap();
    test_free_then_malloc_alignment();

    new_heap();
    test_prev_block_coalescing();

    new_heap();
    test_next_block_coalescing();

    new_heap();
    test_next_and_prev_block_coalescing();

    new_heap();
    test_block_splitting();

    new_heap();
    test_max_size();

    printf("All tests passed!\n");

    return 0;
}
