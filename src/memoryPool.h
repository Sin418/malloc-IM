#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stdbool.h>
#include <stddef.h>

struct MemoryPool {
    void *pool;
    size_t poolsize;
};

struct MetaData {
    int size;
    bool status;
    struct MetaData *front;
    struct MetaData *back;
};

// Initialization functions
int initializePool(struct MemoryPool *mp, size_t size, size_t blockSize);
void initializeMetadata(struct MemoryPool *mp, size_t blockSize);
void clearPool(struct MemoryPool *mp);

// Allocation and deallocation
void* allocateBlock(struct MemoryPool *mp, size_t size);
void deallocateBlock(struct MemoryPool *mp, void *ptr);

// Metadata management
void mergeFreeBlocks(struct MemoryPool *mp, struct MetaData *block);
void* splitBlock(struct MemoryPool *mp, struct MetaData *block, size_t size);

// Block management
struct MetaData* findBlockByPointer(const struct MemoryPool *mp, void *ptr);
struct MetaData* findBlockBySize(const struct MemoryPool *mp, size_t size);
struct MetaData* getFirstFreeBlock(const struct MemoryPool *mp);
struct MetaData* getLastFreeBlock(const struct MemoryPool *mp);

// Pool status and diagnostics
void printPoolStatus(const struct MemoryPool *mp);
void printBlockStatus(const struct MetaData *block);
size_t getTotalFreeSpace(const struct MemoryPool *mp);
size_t getTotalAllocatedSpace(const struct MemoryPool *mp);
size_t getLargestFreeBlockSize(const struct MemoryPool *mp);

// Utility functions
bool isPoolEmpty(const struct MemoryPool *mp);
bool isBlockAllocated(const struct MetaData *block);
bool isBlockFree(const struct MetaData *block);
void printMemoryPoolInfo(const struct MemoryPool *mp);
void validatePool(const struct MemoryPool *mp);

#endif // MEMORY_POOL_H
