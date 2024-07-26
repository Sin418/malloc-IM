#include <stdio.h>
#include <stdbool.h>
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H
 
struct MemoryPool{
    void *pool;
    size_t *poolsize;


};

struct MetaData{
    int size;
    bool status;
    struct MetaData *front;
    struct MetaData *back;

};// Function to initialize the memory pool
int initializePool(struct MemoryPool *mp, size_t size);

// Function to initialize metadata
void initializeMetadata(struct MemoryPool *mp, size_t blockSize);

#endif // MEMORYPOOL_H
