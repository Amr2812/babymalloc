//
// Created by amr on 8/15/24.
//

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "babymalloc.h"

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
    void *ptr = babymalloc(10);
    void *ptr2 = babymalloc(10);
    babyfree(ptr);
    void *ptr3 = babymalloc(16);

    assert(ptr == ptr3);
}

void test_prev_block_coalescing() {
    void *ptr = babymalloc(16);
    void *ptr2 = babymalloc(16);
    babyfree(ptr);
    babyfree(ptr2);
    void *ptr3 = babymalloc(32);

    assert(ptr == ptr3);
}

void test_next_block_coalescing() {
    void *ptr = babymalloc(16);
    void *ptr2 = babymalloc(16);
    babyfree(ptr2);
    babyfree(ptr);
    void *ptr3 = babymalloc(32);

    assert(ptr == ptr3);
}

void test_next_and_prev_block_coalescing() {
    void *ptr = babymalloc(16);
    void *ptr2 = babymalloc(16);
    void *ptr3 = babymalloc(16);
    babyfree(ptr);
    babyfree(ptr3);
    babyfree(ptr2);
    void *ptr4 = babymalloc(48);

    assert(ptr == ptr4);
}

void test_block_splitting() {
    char *ptr = (char *) babymalloc(32);
    babyfree(ptr);
    char *ptr2 = (char *) babymalloc(8);
    char *ptr3 = (char *) babymalloc(8);

    assert(ptr == ptr2);
    assert(ptr3 - ptr2 == 24);
}

void test_max_size() {
    void *ptr = babymalloc(INTPTR_MAX);
    assert(ptr == NULL);
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
