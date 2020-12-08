/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdlib>
#include <algorithm>
#include "repl_policy.h"
#include "cache.h"

AbstarctReplacementPolicy::AbstarctReplacementPolicy(Cache* cache) :
		cache(cache) {
}

/*
 * This is a base function that gets a free block. If there is no free block
 * it will return nullptr which indicates to inherited methods to continue with
 * respective replacement policies.
*/
Block* AbstarctReplacementPolicy::getVictim(uint32_t addr, bool isWrite){
	uint32_t blockAddr = addr / cache->getBlockSize();
	uint64_t setIndex = (cache->getNumSets() - 1) & blockAddr;
	
	for (int i = 0; i < (int) cache->getAssociativity(); i++) {
		if (cache->blocks[setIndex][i]->getValid() == false) {
			return cache->blocks[setIndex][i];
		}
	}
	return nullptr;
}


RandomRepl::RandomRepl(Cache* cache) :
		AbstarctReplacementPolicy(cache) {
}


Block* RandomRepl::getVictim(uint32_t addr, bool isWrite) {
	Block* available = AbstarctReplacementPolicy::getVictim(addr, isWrite);
	if (available) return available;
	
	addr = addr / cache->getBlockSize();
	uint64_t setIndex = (cache->getNumSets() - 1) & addr;

	//randomly choose a block
	int victim_index = rand() % cache->getAssociativity();
	return cache->blocks[setIndex][victim_index];
}

void RandomRepl::update(uint32_t addr, int way, bool isWrite) {
	//there is no metadata to update:D
	return;
}



LRURepl::LRURepl(Cache* cache) :
		AbstarctReplacementPolicy(cache) {
	blockUseLists = (std::vector<Block*>*)malloc(sizeof(std::vector<Block*>*)*cache->getNumSets());
}

LRURepl::~LRURepl(){
	free(blockUseLists);
}


Block* LRURepl::getVictim(uint32_t addr, bool isWrite) {
	// First see if there is an available block
	auto available = AbstarctReplacementPolicy::getVictim(addr, isWrite);
	if (available) return available;	
	
	uint32_t blockAddr = addr / cache->getBlockSize();
	uint64_t setIndex = (cache->getNumSets() - 1) & blockAddr;
	auto blockUseList = blockUseLists[setIndex];

	// Dequeue the front (oldest) used block
	auto staleBlock = blockUseList.front();
	blockUseList.erase(blockUseList.begin());
	return staleBlock;
}

void LRURepl::update(uint32_t addr, int way, bool isWrite) {
	uint32_t blockAddr = addr / cache->getBlockSize();
	uint64_t setIndex = (cache->getNumSets() - 1) & blockAddr;
	auto blockUseList = blockUseLists[setIndex];

	uint32_t tag = addr /(cache->getBlockSize()*cache->getNumSets());

	std::vector<Block*>::iterator foundIterator = std::find_if(blockUseList.begin(),blockUseList.end(),[tag](Block* block){
		return block->getTag() == tag;
	});

	Block* foundBlock;
	if (foundIterator == blockUseList.end()){
		// If not found, add it to the end
		blockUseList.push_back(cache->blocks[setIndex][way]);
		foundBlock = blockUseList.back();
	}else{
		foundBlock = *foundIterator;
		// Move the foundIterator (i.e., vector spot) to the end of the vector
		std::rotate(foundIterator, foundIterator+1, blockUseList.end());
	}
	return;
}