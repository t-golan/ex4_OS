//
// Created by אילון on 23/05/2022.
//

#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <cstdlib>

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
uint64_t findUnusedFrame(uint64_t frameIndex);

/***
 * read the virtual Address and creates a list of the entry in each of the page tables
 * in the tree
 */
void entriesListCreator(uint64_t virtualAddress, int* listOfEntries);

void extractPageNumber(uint64_t virtualAddress, uint64_t* pageNumber){
    *pageNumber = virtualAddress >> OFFSET_WIDTH;
}

/**
 * clears a given frame
 * @param frameIndex
 */
void clearFrame(uint64_t frameIndex);

/**
 *
 * @param pageSwappedIn the page number that should be swapped in
 * @return the frame index of the page we want to evict
 */
uint64_t findFrameToEvict(uint64_t* pageSwappedIn);

/**
 * find the index of the frame where the value in the given address is saved
 * @param virtualAddress the address
 * @param entriesList the entries in the tables that leads to the wanted frame
 */
uint64_t searchForthePageFrame(uint64_t virtualAddress, int* entriesList);

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
         uint64_t* maxFrameIndex, int depth){

    if(depth > TABLES_DEPTH){
        return;
    }
    if(frameIsEmpty(frameIndex)){
        *emptyFrameIndex = frameIndex;
        return;
    }
    iterateFrameWithDfs(frameIndex, emptyFrameIndex, maxFrameIndex, depth);
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

uint64_t findUnusedFrame(uint64_t frameIndex){
    uint64_t headFrame = 0;
    uint64_t emptyFrameIndex = 0;
    uint64_t maxFrameIndex = 0;
    uint64_t  maxDistanceFrame = 0;
    uint64_t pageNumber = 0;

    int maxDistance = 0;
    int depth = 1;

    dfs(headFrame, &emptyFrameIndex, &maxFrameIndex, depth);
    if(emptyFrameIndex != 0 && emptyFrameIndex != frameIndex){
        return emptyFrameIndex;
    }
    if(maxFrameIndex + 1 < NUM_FRAMES){
        return maxFrameIndex + 1;
    }
    return 0;
}

uint64_t findFrameToEvict(uint64_t* pageSwappedIn){
    exit(1);
}

void entriesListCreator(uint64_t virtualAddress, int* listOfEntries){

    int headOfSet = VIRTUAL_ADDRESS_WIDTH % OFFSET_WIDTH;
    if(headOfSet == 0){
        headOfSet = OFFSET_WIDTH;
    }
    for(int i=0; i < TABLES_DEPTH + 1; i++){
        listOfEntries[i] = virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - (headOfSet + i * OFFSET_WIDTH));
        virtualAddress = (virtualAddress << (headOfSet + i * OFFSET_WIDTH)) & ((1 << VIRTUAL_ADDRESS_WIDTH) - 1);
        virtualAddress = virtualAddress >> (headOfSet + i * OFFSET_WIDTH);
    }
}

void clearFrame(uint64_t frameIndex){
    for(int i=0; i < PAGE_SIZE; i++){
        PMwrite(frameIndex*PAGE_SIZE + i,  0);
    }
}

uint64_t searchForthePageFrame(uint64_t virtualAddress, int* entriesList){
    uint64_t pageNumber = 0;
    extractPageNumber(virtualAddress, &pageNumber);

    int d = 0;
    word_t frameIndex = 0;
    word_t value;

    while(d < TABLES_DEPTH){
        PMread(PAGE_SIZE * frameIndex + entriesList[d], &value);
        if (value == 0) {
            uint64_t newFrameIndex = findUnusedFrame(frameIndex);
            if (newFrameIndex == 0) {
                newFrameIndex = findFrameToEvict(&pageNumber);
            }
            clearFrame(newFrameIndex);
            PMwrite(PAGE_SIZE * frameIndex + entriesList[d], (word_t)newFrameIndex);
            frameIndex = (word_t)newFrameIndex;
        }
        else{
            frameIndex = value;
        }
        d++;
    }
    return (uint64_t)frameIndex;
}

void VMinitialize(){
    clearFrame(0);
}

int VMread(uint64_t virtualAddress, word_t* value){
    int entriesInPageTables[TABLES_DEPTH + 1];
    entriesListCreator(virtualAddress, entriesInPageTables);
    uint64_t frameIndex = searchForthePageFrame(virtualAddress, entriesInPageTables);
    PMread(PAGE_SIZE*frameIndex + entriesInPageTables[TABLES_DEPTH], value);

}

int VMwrite(uint64_t virtualAddress, word_t value){
    int entriesInPageTables[TABLES_DEPTH + 1];
    entriesListCreator(virtualAddress, entriesInPageTables);
    uint64_t frameIndex = searchForthePageFrame(virtualAddress, entriesInPageTables);
    PMwrite(PAGE_SIZE*frameIndex + entriesInPageTables[TABLES_DEPTH], value);
}
