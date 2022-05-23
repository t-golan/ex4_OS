//
// Created by אילון on 23/05/2022.
//

#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <cstdlib>
#include <algorithm>
using namespace std;

/**
 * checks if the frame is empty
 * @param frameIndex the index of the frame
 * @return true if its empty
 */
bool frameIsEmpty(uint64_t frameIndex);

/**
 * search the sub-tree for an empty frame - recursive
 * @param frameIndex the current frame (head of subtree)
 * @param emptyFrameIndex the index of an empty frame - if one exist
 * @param maxFrameIndex - the max index of frame pointed by all page tables
 * @param depth the depth of the recursive algo
 */
void dfs(uint64_t frameIndex, uint64_t* emptyFrameIndex,
         uint64_t* maxFrameIndex, int depth);
/**
* a helper method to be used by the dfs algo
* @param frameIndex the current frame (head of subtree)
* @param emptyFrameIndex the index of an empty frame - if one exist
* @param maxFrameIndex - the max index of frame pointed by all page tables
* @param depth the depth of the recursive algo
 * */
void iterateFrameWithDfs(uint64_t frameIndex, uint64_t* emptyFrameIndex,
         uint64_t* maxFrameIndex, int depth);
/**
 * find an empty frame either by finding the max index of all frames or by finding an empty frame
 * if none of the frame is empty return 0
 * @return an empty frame index - if no frame is empty return 0
 */
uint64_t findUnusedFrame();

/**
 *
 * @param pageSwappedIn the page number that should be swapped in
 * @return the frame index of the page we want to evict
 */
uint64_t findFrameToEvict(uint64_t* pageSwappedIn);


bool frameIsEmpty(uint64_t frameIndex){
    word_t value = 0;

    for(uint64_t  i = 0; i < PAGE_SIZE; i++){
        PMread(frameIndex * PAGE_SIZE +  i, &value);
        if(value != 0){
            return false;
        }
    }
    return true;
}

void dfs(uint64_t frameIndex, uint64_t* emptyFrameIndex,
         uint64_t* maxFrameIndex, uint64_t* maxDistanceFrame, int* maxDistance,
         uint64_t* pageNumber,
         int depth){

    if(depth > TABLES_DEPTH){
        return;
    }
    if(frameIsEmpty(frameIndex)){
        *emptyFrameIndex = frameIndex;
        return;
    }
    iterateFrameWithDfs(frameIndex, emptyFrameIndex,
                        maxFrameIndex, depth);
}


void iterateFrameWithDfs(uint64_t frameIndex, uint64_t* emptyFrameIndex,
                         uint64_t* maxFrameIndex, int depth){
    word_t value = 0;
    int offset = OFFSET_WIDTH;

    if(depth == 1){
        offset = VIRTUAL_ADDRESS_WIDTH % OFFSET_WIDTH;
    }

    for(uint64_t  i = 0; i < PAGE_SIZE; i++){
        PMread(frameIndex * PAGE_SIZE + i, &value);
        uint64_t newFrameIndex = value;
        if (newFrameIndex != 0) {
            if (*maxFrameIndex < newFrameIndex){
                *maxFrameIndex = newFrameIndex;
            }
            dfs(newFrameIndex, emptyFrameIndex,
                maxFrameIndex, depth + 1);
        }
    }
}

uint64_t findUnusedFrame(){
    uint64_t headFrame = 0;
    uint64_t emptyFrameIndex = 0;
    uint64_t maxFrameIndex = 0;
    uint64_t  maxDistanceFrame = 0;
    uint64_t pageNumber = 0;

    int maxDistance = 0;
    int depth = 1;

    dfs(headFrame, &emptyFrameIndex, &maxFrameIndex,
            &maxDistanceFrame, &maxDistance, &pageNumber, depth);
    if(emptyFrameIndex != 0){
        return emptyFrameIndex;
    }
    if(maxFrameIndex + 1 < NUM_FRAMES){
        return maxFrameIndex + 1;
    }
    return 0;
}

uint64_t findFrameToEvict(uint64_t* pageSwappedIn){
    return 0;
}
