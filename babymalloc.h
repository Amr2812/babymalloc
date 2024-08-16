#ifndef BABYMALLOC_BABYMALLOC_H
#define BABYMALLOC_BABYMALLOC_H

#include <stdlib.h>

void *babymalloc(size_t size);
void babyfree(void *ptr);

#endif //BABYMALLOC_BABYMALLOC_H
