/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdlib>
#include <algorithm>
#include <numeric>
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
	auto setIndex = cache->getLocation(addr).set;
	
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

	auto setIndex = cache->getLocation(addr).set;

	//randomly choose a block
	int victim_index = rand() % cache->getAssociativity();
	return cache->blocks[setIndex][victim_index];
}

void RandomRepl::update(uint32_t addr, int way, bool isWrite) {
	//there is no metadata to update:D
	return;
}

LRURepl::LRURepl(Cache* cache):
	AbstarctReplacementPolicy(cache){
	
	// Create ordering the size of associativity
	Ordering defaultO(cache->getAssociativity());
	// Fill 0, 1, 2... 
	std::iota(defaultO.begin(),defaultO.end(),0);

	useTracker = std::vector<Ordering>(cache->getNumSets(), defaultO);	
}

Block* LRURepl::getVictim(uint32_t addr, bool isWrite){
	Block* available = AbstarctReplacementPolicy::getVictim(addr, isWrite);
	if (available) return available;

	auto setIndex = cache->getLocation(addr).set;
	auto ordering = useTracker[setIndex];
	auto way = *ordering.begin();
	// Send the oldest way to the back
	std::rotate(ordering.begin(),ordering.begin()+1, ordering.end());
	return cache->blocks[setIndex][way];
}

void LRURepl::update(uint32_t addr, int way, bool isWrite){
	auto set = addr/cache->getBlockSize();
	auto ordering = useTracker[cache->getLocation(addr).set];
	auto wayIterator = std::find(ordering.begin(),ordering.end(),way);
	if (wayIterator == ordering.end()){
		assert(false && "Error, way greater than expected");
	}else{
		// Move the way to the back
		std::rotate(wayIterator,wayIterator+1, ordering.end());
	}
}

PLRURepl::PLRURepl(Cache* cache):AbstarctReplacementPolicy(cache){
	auto intLog2 = [](uint32_t number){
		uint32_t count = 0;
		while (number >>= 1) count++;
		return count;
	};

	// There is no end node for 
	auto treesize = intLog2(cache->getAssociativity())-1;
	auto defaultTree = TreeArray(treesize,true);
	useTracker = std::vector<TreeArray>(cache->getNumSets());
}

Block* PLRURepl::getVictim(uint32_t addr, bool isWrite){
	Block* available = AbstarctReplacementPolicy::getVictim(addr, isWrite);
	if (available) return available;

	auto setIndex = cache->getLocation(addr).set;

	auto tree = useTracker[setIndex];

	auto i = 0;
	while(i<tree.size()){
		i = tree[i]?
				i*2+1 // True (right branches)
				:i*2;  // false (left branches)
	};
	// Once we go over the size of the array, we are now in the "leaves" that aren't really
	// represented in the array. 
	auto way = i-tree.size();

	// Update boolean flags
	update(addr,way,isWrite);
	return cache->blocks[setIndex][i];
}

void PLRURepl::update(uint32_t addr, int way, bool isWrite){
	auto setIndex = cache->getLocation(addr).set;
	auto tree = useTracker[setIndex];
	way += tree.size();
	way>>1;
	while (way){
		tree[way] = !tree[way];
		way>>1;
	}
}