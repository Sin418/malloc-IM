#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Define constants for alignment and safety
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define POOL_MAGIC 0xDEADBEEF

// Error codes
#define POOL_SUCCESS 0
#define POOL_ERROR_NULL 1
#define POOL_ERROR_OOM 2
#define POOL_ERROR_INVALID 3

struct MemoryPool {
    void *pool;
    size_t poolsize;
    size_t blockSize;
    size_t allocatedSize;
    struct MetaData *freeList;
};

struct MetaData {
    size_t size;
    bool status;
    uint32_t magic;
    struct MetaData *prev;
    struct MetaData *next;
};

// Initialization functions
int initializePool(struct MemoryPool *mp, size_t size, size_t blockSize);
void clearPool(struct MemoryPool *mp);

// Allocation and deallocation
void* allocateBlock(struct MemoryPool *mp, size_t size);
int deallocateBlock(struct MemoryPool *mp, void *ptr);

// Metadata management
void mergeFreeBlocks(struct MemoryPool *mp, struct MetaData *block);
void* splitBlock(struct MemoryPool *mp, struct MetaData *block, size_t size);
struct MetaData* findBlockByPointer(const struct MemoryPool *mp, void *ptr);
struct MetaData* findBlockBySize(const struct MemoryPool *mp, size_t size);

// Advanced Allocation Strategies
void* allocateBestFit(struct MemoryPool *mp, size_t size);
void* allocateWorstFit(struct MemoryPool *mp, size_t size);
void* allocateFirstFit(struct MemoryPool *mp, size_t size);

// Pool status and diagnostics
void printPoolStatus(const struct MemoryPool *mp);
void printBlockStatus(const struct MetaData *block);
size_t getTotalFreeSpace(const struct MemoryPool *mp);
size_t getTotalAllocatedSpace(const struct MemoryPool *mp);
size_t getLargestFreeBlockSize(const struct MemoryPool *mp);

// Advanced Utility Functions
bool isPoolCorrupted(const struct MemoryPool *mp);
void defragmentPool(struct MemoryPool *mp);
void validatePool(const struct MemoryPool *mp);

// Utility functions
bool isPoolEmpty(const struct MemoryPool *mp);
bool isBlockAllocated(const struct MetaData *block);
bool isBlockFree(const struct MetaData *block);
void printMemoryPoolInfo(const struct MemoryPool *mp);

#endif // MEMORY_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryPool.h"

// Aligns the memory pool size and block size to prevent alignment issues
size_t alignSize(size_t size) {
    return ALIGN(size);
}

int initializePool(struct MemoryPool *mp, size_t size, size_t blockSize) {
    if (!mp) return POOL_ERROR_NULL;

    mp->pool = malloc(size);
    if (!mp->pool) return POOL_ERROR_OOM;

    mp->poolsize = size;
    mp->blockSize = alignSize(blockSize);
    mp->allocatedSize = 0;

    mp->freeList = (struct MetaData *)mp->pool;
    mp->freeList->size = size;
    mp->freeList->status = false;
    mp->freeList->magic = POOL_MAGIC;
    mp->freeList->prev = NULL;
    mp->freeList->next = NULL;

    return POOL_SUCCESS;
}

void clearPool(struct MemoryPool *mp) {
    if (!mp || !mp->pool) return;

    free(mp->pool);
    mp->pool = NULL;
    mp->poolsize = 0;
    mp->blockSize = 0;
    mp->allocatedSize = 0;
    mp->freeList = NULL;
}

void* allocateBlock(struct MemoryPool *mp, size_t size) {
    return allocateFirstFit(mp, size);
}

int deallocateBlock(struct MemoryPool *mp, void *ptr) {
    if (!mp || !ptr) return POOL_ERROR_NULL;

    struct MetaData *block = findBlockByPointer(mp, ptr);
    if (!block || block->magic != POOL_MAGIC) return POOL_ERROR_INVALID;

    block->status = false;
    mp->allocatedSize -= block->size;
    mergeFreeBlocks(mp, block);
    return POOL_SUCCESS;
}

void* allocateFirstFit(struct MemoryPool *mp, size_t size) {
    if (!mp) return NULL;

    struct MetaData *current = mp->freeList;
    while (current) {
        if (!current->status && current->size >= size) {
            if (current->size > size + sizeof(struct MetaData)) {
                return splitBlock(mp, current, size);
            } else {
                current->status = true;
                mp->allocatedSize += current->size;
                return (void *)((char *)current + sizeof(struct MetaData));
            }
        }
        current = current->next;
    }
    return NULL;
}

void* allocateBestFit(struct MemoryPool *mp, size_t size) {
    if (!mp) return NULL;

    struct MetaData *bestBlock = NULL;
    struct MetaData *current = mp->freeList;
    while (current) {
        if (!current->status && current->size >= size) {
            if (!bestBlock || current->size < bestBlock->size) {
                bestBlock = current;
            }
        }
        current = current->next;
    }

    if (bestBlock) {
        if (bestBlock->size > size + sizeof(struct MetaData)) {
            return splitBlock(mp, bestBlock, size);
        } else {
            bestBlock->status = true;
            mp->allocatedSize += bestBlock->size;
            return (void *)((char *)bestBlock + sizeof(struct MetaData));
        }
    }

    return NULL;
}

void* allocateWorstFit(struct MemoryPool *mp, size_t size) {
    if (!mp) return NULL;

    struct MetaData *worstBlock = NULL;
    struct MetaData *current = mp->freeList;
    while (current) {
        if (!current->status && current->size >= size) {
            if (!worstBlock || current->size > worstBlock->size) {
                worstBlock = current;
            }
        }
        current = current->next;
    }

    if (worstBlock) {
        if (worstBlock->size > size + sizeof(struct MetaData)) {
            return splitBlock(mp, worstBlock, size);
        } else {
            worstBlock->status = true;
            mp->allocatedSize += worstBlock->size;
            return (void *)((char *)worstBlock + sizeof(struct MetaData));
        }
    }

    return NULL;
}

void* splitBlock(struct MemoryPool *mp, struct MetaData *block, size_t size) {
    size_t remainingSize = block->size - size - sizeof(struct MetaData);

    if (remainingSize >= sizeof(struct MetaData)) {
        struct MetaData *newBlock = (struct MetaData *)((char *)block + sizeof(struct MetaData) + size);
        newBlock->size = remainingSize;
        newBlock->status = false;
        newBlock->magic = POOL_MAGIC;
        newBlock->prev = block;
        newBlock->next = block->next;

        if (block->next) {
            block->next->prev = newBlock;
        }

        block->next = newBlock;
        block->size = size;
    }

    block->status = true;
    mp->allocatedSize += block->size;
    return (void *)((char *)block + sizeof(struct MetaData));
}

void mergeFreeBlocks(struct MemoryPool *mp, struct MetaData *block) {
    if (block->next && !block->next->status) {
        block->size += block->next->size + sizeof(struct MetaData);
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    if (block->prev && !block->prev->status) {
        block->prev->size += block->size + sizeof(struct MetaData);
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

struct MetaData* findBlockByPointer(const struct MemoryPool *mp, void *ptr) {
    if (!mp || !ptr) return NULL;

    struct MetaData *current = mp->freeList;
    while (current) {
        if ((void *)((char *)current + sizeof(struct MetaData)) == ptr) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct MetaData* findBlockBySize(const struct MemoryPool *mp, size_t size) {
    if (!mp) return NULL;

    struct MetaData *current = mp->freeList;
    while (current) {
        if (!current->status && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

bool isPoolCorrupted(const struct MemoryPool *mp) {
    if (!mp) return true;

    struct MetaData *current = mp->freeList;
    while (current) {
        if (current->magic != POOL_MAGIC) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void defragmentPool(struct MemoryPool *mp) {
    if (!mp) return;

    struct MetaData *current = mp->freeList;
    while (current && current->next) {
        if (!current->status && !current->next->status) {
            mergeFreeBlocks(mp, current);
        }
        current = current->next;
    }
}

void printPoolStatus(const struct MemoryPool *mp) {
    if (!mp) return;

    struct MetaData *current = mp->freeList;
    size_t blockCount = 0;

    while (current) {
        printf("Block %zu: Size = %zu, Status = %s, Magic = 0x%X\n",
               blockCount, current->size, current->status ? "Allocated" : "Free", current->magic);
        current = current->next;
        blockCount++;
    }
}

void printBlockStatus(const struct MetaData *block) {
    if (!block) return;

    printf("Block Size = %zu, Status = %s, Magic = 0x%X\n",
           block->size, block->status ? "Allocated" : "Free", block->magic);
}

size_t getTotalFreeSpace(const struct MemoryPool *mp) {
    if (!mp) return 0;

    struct MetaData *current = mp->freeList;
    size_t totalFree = 0;

    while (current) {
        if (!current->status) {
            totalFree += current->size;
        }
        current = current->next;
    }
    return totalFree;
}

size_t getTotalAllocatedSpace(const struct MemoryPool *mp) {
    if (!mp) return 0;

    return mp->allocatedSize;
}

size_t getLargestFreeBlockSize(const struct MemoryPool *mp) {
    if (!mp) return 0;

    struct MetaData *current = mp->freeList;
    size_t largestSize = 0;

    while (current) {
        if (!current->status && current->size > largestSize) {
            largestSize = current->size;
        }
        current = current->next;
    }
    return largestSize;
}

bool isPoolEmpty(const struct MemoryPool *mp) {
    if (!mp) return true;

    return mp->allocatedSize == 0;
}

bool isBlockAllocated(const struct MetaData *block) {
    if (!block) return false;

    return block->status;
}

bool isBlockFree(const struct MetaData *block) {
    if (!block) return false;

    return !block->status;
}

void printMemoryPoolInfo(const struct MemoryPool *mp) {
    if (!mp) return;

    printf("Total Pool Size: %zu bytes\n", mp->poolsize);
    printf("Total Free Space: %zu bytes\n", getTotalFreeSpace(mp));
    printf("Total Allocated Space: %zu bytes\n", getTotalAllocatedSpace(mp));
    printf("Largest Free Block Size: %zu bytes\n", getLargestFreeBlockSize(mp));
    printf("Pool is %s\n", isPoolCorrupted(mp) ? "Corrupted" : "Healthy");
}

void validatePool(const struct MemoryPool *mp) {
    if (!mp) return;

    struct MetaData *current = mp->freeList;
    while (current) {
        if (current->magic != POOL_MAGIC) {
            printf("Pool validation failed: Block magic mismatch\n");
            return;
        }
        current = current->next;
    }
    printf("Pool validation succeeded\n");
}
