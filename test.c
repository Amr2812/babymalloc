//
// Created by amr on 8/15/24.
//

#include <assert.h>

#include "babymalloc.h"

void test_free_then_malloc_alignment() {
    void *ptr = babymalloc(10);
    print_heap();
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

int main(int argc, char **argv) {
//    test_free_then_malloc_alignment();
//    test_prev_block_coalescing();
//    test_next_block_coalescing();
    test_next_and_prev_block_coalescing();

    return 0;
}
