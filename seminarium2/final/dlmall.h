#include <stddef.h>
void *dalloc(size_t request);
void dfree(void *memory);
void sanity();
void traverse();
void init();
void block_sizes(int max, int *sizes);
void flist_size(unsigned int *size);