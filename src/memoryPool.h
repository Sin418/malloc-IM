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

/**
 * @brief Initializes the memory pool.
 * 
 * @param mp Pointer to the MemoryPool structure.
 * @param size Total size of the memory pool in bytes.
 * @param blockSize Size of each block in the pool.
 * @return 1 if initialization is successful, 0 otherwise.
 */
int initializePool(struct MemoryPool *mp, size_t size, size_t blockSize);

/**
 * @brief Initializes the metadata for the memory pool.
 * 
 * @param mp Pointer to the MemoryPool structure.
 * @param blockSize Size of each block in the pool.
 */
void initializeMetadata(struct MemoryPool *mp, size_t blockSize);

/**
 * @brief Clears the memory pool.
 * 
 * @param mp Pointer to the MemoryPool structure.
 */
void clearPool(struct MemoryPool *mp);

/**
 * @brief Allocates a block of memory from the pool.
 * 
 * @param mp Pointer to the MemoryPool structure.
 * @param size Size of the block to allocate.
 * @return Pointer to the allocated block or NULL if allocation fails.
 */
void* allocateBlock(struct MemoryPool *mp, size_t size);

/**
 * @brief Frees a previously allocated block of memory.
 * 
 * @param mp Pointer to the MemoryPool structure.
 * @param ptr Pointer to the block to free.
 */
void deallocateBlock(struct MemoryPool *mp, void *ptr);

/**
 * @brief Prints the status of all blocks in the memory pool.
 * 
 * @param mp Pointer to the MemoryPool structure.
 */
void printPoolStatus(const struct MemoryPool *mp);

/**
 * @brief Merges adjacent free blocks to form a larger free block.
 * 
 * @param mp Pointer to the MemoryPool structure.
 * @param block Pointer to the block to start merging from.
 */
void mergeFreeBlocks(struct MemoryPool *mp, struct MetaData *block);

/**
 * @brief Splits a block into smaller blocks.
 * 
 * @param mp Pointer to the MemoryPool structure.
 * @param block Pointer to the block to split.
 * @param size Size of the new smaller block.
 * @return Pointer to the newly created smaller block or NULL if splitting fails.
 */
void* splitBlock(struct MemoryPool *mp, struct MetaData *block, size_t size);

#endif // MEMORY_POOL_H
