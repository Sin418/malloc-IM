#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryPool.h"

void initializeMetadata(struct MemoryPool *mp, size_t blockSize) {
    size_t numBlocks = mp->poolsize / blockSize;
    char *currentPtr = (char *)mp->pool;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        currentObj->size = blockSize;
        currentObj->status = false;
        if (i == 0) {
            currentObj->back = NULL;
        } else {
            currentObj->back = (struct MetaData *)(currentPtr - blockSize);
        }
        if (i == numBlocks - 1) {
            currentObj->front = NULL;
        } else {
            currentObj->front = (struct MetaData *)(currentPtr + blockSize);
        }
        currentPtr += blockSize;
    }
}

int initializePool(struct MemoryPool *mp, size_t size, size_t blockSize) {
    mp->pool = malloc(size);
    if (mp->pool == NULL) {
        return 0;
    }
    mp->poolsize = size;
    initializeMetadata(mp, blockSize);
    return 1;
}

void clearPool(struct MemoryPool *mp) {
    if (mp->pool != NULL) {
        free(mp->pool);
        mp->pool = NULL;
    }
}

void* allocateBlock(struct MemoryPool *mp, size_t size) {
    char *currentPtr = (char *)mp->pool;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if (!currentObj->status && currentObj->size >= size) {
            if (currentObj->size > size + sizeof(struct MetaData)) {
                return splitBlock(mp, currentObj, size);
            } else {
                currentObj->status = true;
                return (void *)(currentPtr + sizeof(struct MetaData));
            }
        }
        currentPtr += currentObj->size;
    }
    return NULL;
}

void deallocateBlock(struct MemoryPool *mp, void *ptr) {
    char *blockPtr = (char *)ptr - sizeof(struct MetaData);
    struct MetaData *block = (struct MetaData *)blockPtr;
    block->status = false;
    mergeFreeBlocks(mp, block);
}

void mergeFreeBlocks(struct MemoryPool *mp, struct MetaData *block) {
    while (block->back && !block->back->status) {
        block->back->size += block->size;
        block->back->front = block->front;
        if (block->front) {
            block->front->back = block->back;
        }
        block = block->back;
    }
    while (block->front && !block->front->status) {
        block->size += block->front->size;
        block->front = block->front->front;
        if (block->front) {
            block->front->back = block;
        }
    }
}

void* splitBlock(struct MemoryPool *mp, struct MetaData *block, size_t size) {
    size_t remainingSize = block->size - size - sizeof(struct MetaData);
    if (remainingSize >= sizeof(struct MetaData)) {
        struct MetaData *newBlock = (struct MetaData *)((char *)block + sizeof(struct MetaData) + size);
        newBlock->size = remainingSize;
        newBlock->status = false;
        newBlock->front = block->front;
        newBlock->back = block;
        if (block->front) {
            block->front->back = newBlock;
        }
        block->front = newBlock;
        block->size = size;
    }
    block->status = true;
    return (void *)((char *)block + sizeof(struct MetaData));
}

struct MetaData* findBlockByPointer(const struct MemoryPool *mp, void *ptr) {
    char *currentPtr = (char *)mp->pool;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if ((void *)(currentPtr + sizeof(struct MetaData)) == ptr) {
            return currentObj;
        }
        currentPtr += currentObj->size;
    }
    return NULL;
}

struct MetaData* findBlockBySize(const struct MemoryPool *mp, size_t size) {
    char *currentPtr = (char *)mp->pool;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if (!currentObj->status && currentObj->size >= size) {
            return currentObj;
        }
        currentPtr += currentObj->size;
    }
    return NULL;
}

struct MetaData* getFirstFreeBlock(const struct MemoryPool *mp) {
    char *currentPtr = (char *)mp->pool;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if (!currentObj->status) {
            return currentObj;
        }
        currentPtr += currentObj->size;
    }
    return NULL;
}

struct MetaData* getLastFreeBlock(const struct MemoryPool *mp) {
    char *currentPtr = (char *)mp->pool + mp->poolsize - sizeof(struct MetaData);
    size_t blockSize = ((struct MetaData *)currentPtr)->size;

    while (currentPtr >= (char *)mp->pool) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if (!currentObj->status) {
            return currentObj;
        }
        currentPtr -= blockSize;
    }
    return NULL;
}

void printPoolStatus(const struct MemoryPool *mp) {
    char *currentPtr = (char *)mp->pool;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        printf("Block %zu: Size = %d, Status = %s\n", i, currentObj->size, currentObj->status ? "Allocated" : "Free");
        currentPtr += currentObj->size;
    }
}

void printBlockStatus(const struct MetaData *block) {
    printf("Size = %d, Status = %s\n", block->size, block->status ? "Allocated" : "Free");
}

size_t getTotalFreeSpace(const struct MemoryPool *mp) {
    char *currentPtr = (char *)mp->pool;
    size_t totalFree = 0;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if (!currentObj->status) {
            totalFree += currentObj->size;
        }
        currentPtr += currentObj->size;
    }
    return totalFree;
}

size_t getTotalAllocatedSpace(const struct MemoryPool *mp) {
    return mp->poolsize - getTotalFreeSpace(mp);
}

size_t getLargestFreeBlockSize(const struct MemoryPool *mp) {
    char *currentPtr = (char *)mp->pool;
    size_t largestSize = 0;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if (!currentObj->status && currentObj->size > largestSize) {
            largestSize = currentObj->size;
        }
        currentPtr += currentObj->size;
    }
    return largestSize;
}

bool isPoolEmpty(const struct MemoryPool *mp) {
    return getTotalFreeSpace(mp) == mp->poolsize;
}

bool isBlockAllocated(const struct MetaData *block) {
    return block->status;
}

bool isBlockFree(const struct MetaData *block) {
    return !block->status;
}

void printMemoryPoolInfo(const struct MemoryPool *mp) {
    printf("Total Pool Size: %zu bytes\n", mp->poolsize);
    printf("Total Free Space: %zu bytes\n", getTotalFreeSpace(mp));
    printf("Total Allocated Space: %zu bytes\n", getTotalAllocatedSpace(mp));
    printf("Largest Free Block Size: %zu bytes\n", getLargestFreeBlockSize(mp));
}

void validatePool(const struct MemoryPool *mp) {
    char *currentPtr = (char *)mp->pool;
    size_t numBlocks = mp->poolsize / ((struct MetaData *)currentPtr)->size;

    for (size_t i = 0; i < numBlocks; i++) {
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        if (currentObj->front && (char *)currentObj->front != currentPtr + currentObj->size) {
            printf("Pool validation failed: Block %zu front pointer mismatch\n", i);
            return;
        }
        if (currentObj->back && (char *)currentObj->back != currentPtr - currentObj->size) {
            printf("Pool validation failed: Block %zu back pointer mismatch\n", i);
            return;
        }
        currentPtr += currentObj->size;
    }
    printf("Pool validation succeeded\n");
}
