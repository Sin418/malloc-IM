#include <stdio.h>
#include <stdlib.h>
#include "memoryPool.h"


void initializeMetadata(struct MemoryPool *mp, size_t blockSize ){
    size_t numBlocks = mp->poolsize/blockSize;
    // current type = void, cant do byte opperations on it, need to cast to char for 1 byte
    char *currentPtr = (char *)mp->pool;
    
    // looping to create blocks inside pool
    for(int i =0;i<numBlocks;i++){
        struct MetaData *currentObj = (struct MetaData *)currentPtr;
        currentObj->size = blockSize;
        currentObj->status = 0;
        if (i==0){
            currentObj->back = NULL;
        } else{
            currentObj->back = (struct MetaData *)(currentPtr - blockSize);
        }
        if (i == numBlocks-1){
            currentObj->front = NULL;

        } else{
            currentObj->front = (struct MetaData *)(currentPtr + blockSize);

        } 
        currentPtr += blockSize;
        
    }

}



int initializePool(struct MemoryPool *mp, size_t size){
    mp->pool = malloc(size);
    // checking if malloc failed
    if(mp->pool == NULL){
        printf("Error while allocating memory to pool");
        return 0;

    }
    printf("memory allocated of size: %zu",size);
    initializeMetadata(mp,512);
    return 1;

} 
