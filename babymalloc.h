#ifndef BABYMALLOC_BABYMALLOC_H
#define BABYMALLOC_BABYMALLOC_H

#include <stdlib.h>

void *babymalloc(size_t size);
void babyfree(void *ptr);
void print_heap();
void new_heap();

#endif //BABYMALLOC_BABYMALLOC_H
