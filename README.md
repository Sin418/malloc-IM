# Memory Pool Allocator

## Overview

This project implements a simple memory pool allocator in C. It provides a mechanism to manage memory efficiently by allocating a large block of memory and dividing it into smaller, fixed-size blocks. This can help reduce fragmentation and improve performance in scenarios with frequent memory allocations and deallocations.

## Features

- Efficient Memory Management: Allocate and manage memory using a fixed-size block strategy.
- Metadata Tracking: Keep track of memory block status and neighboring blocks.

## Functions

- `initializePool`: Allocates memory for the pool and sets up metadata.
- `initializeMetadata`: Sets up metadata for each block in the pool.
- `clearPool`: Frees the allocated memory and resets the pool pointer.

## Compilation

To compile the source code, use:

```bash
gcc -o memoryPool memoryPool.c
