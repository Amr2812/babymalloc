//
// Created by amr on 8/15/24.
//

#include <assert.h>

#include "babymalloc.h"

void testFreeThenMallocAlignment() {
    void *ptr = babymalloc(10);
    void *ptr2 = babymalloc(10);
    babyfree(ptr);
    void *ptr3 = babymalloc(16);

    assert(ptr == ptr3);
}

int main(int argc, char **argv) {
    testFreeThenMallocAlignment();

    return 0;
}
