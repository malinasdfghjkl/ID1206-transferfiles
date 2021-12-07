#include <stddef.h>
void *dalloc(size_t request);
void dfree(void *memory);
void sanity();
void traverse();
void init();
void block_sizes(int max, int *sizes);
int flist_size(unsigned int *size, int numberofblocks);
//void benchmark(int howMany, int requested, int min, int max);
//int request(unsigned int min, unsigned int max);