/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include "repl_policy.h"
#include "next_line_prefetcher.h"
#include "cache.h"

Cache::Cache(CACHE_PARAMS_TYPED):
		// Initialize parent
		AbstractMemory(delay, 100),cSize(size),
				associativity(associativity), blkSize(blkSize){

	numSets = cSize / (blkSize * associativity);
	blocks = new Block**[numSets];

	auto blankSubentries = std::vector<MSHRSubEntry>(mshr_subentries, (MSHRSubEntry) {false, nullptr});
	mshr = std::vector<MSHREntry>(mshr_entries,(MSHREntry){false, 0, false, blankSubentries});

	for (int i = 0; i < (int) numSets; i++) {
		blocks[i] = new Block*[associativity];
		for (int j = 0; j < associativity; j++)
			blocks[i][j] = new Block(blkSize);
	}

	// Set up replacement policy
	switch (replacementPolicy) {
		case RandomReplPolicy:
			replPolicy = new RandomRepl(this);
			break;
		case LRUReplPolicy:
			replPolicy = new LRURepl(this);
			break;
		case PLRUReplPolicy:
			replPolicy = new PLRURepl(this);
			break;
		default:
			assert(false && "Unknown Replacement Policy");
	}

	// Defaults and errata	
	next == nullptr;
	prev == nullptr;

	if (label.length()==0) label = "Cache";
}

Cache::~Cache() {
	delete replPolicy;
	for (int i = 0; i < (int) numSets; i++) {
		for (int j = 0; j < associativity; j++)
			delete blocks[i][j];
		delete blocks[i];
	}
	delete blocks;
	delete replPolicy;
}

uint32_t Cache::getAssociativity() {
	return associativity;
}

uint32_t Cache::getNumSets() {
	return numSets;
}

uint32_t Cache::getBlockSize() {
	return blkSize;
}


Location Cache::getLocation(uint32_t addr){
	uint32_t _addr = addr / blkSize;
	uint32_t setIndex = (numSets - 1) & _addr;
	uint32_t addrTag = _addr / numSets;
	uint32_t offset = addr % blkSize;
	return {addrTag,setIndex,offset};
};

uint32_t Cache::getAddress(Location location){
	uint32_t address = location.tag;
	address *= numSets;
	address += location.set;
	address *= blkSize;
	address += location.offset;
	return address;
};

int Cache::getWay(uint32_t addr) {
	Location location = getLocation(addr);
	for (int i = 0; i < (int) associativity; i++) {
		if ((blocks[location.set][i]->getValid() == true)
				&& (blocks[location.set][i]->getTag() == location.tag)) {
			return i;
		}
	}
	return -1;
}

/**
 * Creates a copy of the data on the heap and returns it
*/
uint8_t* duplicateData(uint8_t* source, uint32_t numberItems){
	uint8_t* dest = new uint8_t[numberItems];
	std::memcpy(dest,source,numberItems*sizeof(uint8_t));
	for(uint32_t i = 0; i<numberItems;i++){
		dest[i] = source[i];
	}
	return dest;
}

/**
 * Copies data from source to destination, does not create new data
*/
void overwriteData(uint8_t* dest, uint8_t* source, uint32_t numberItems){
	std::memcpy(dest,source,numberItems*sizeof(uint8_t));
	for(uint32_t i = 0; i<numberItems;i++){
		dest[i] = source[i];
	}
} 

/**
 * This function should be called when this cache is to receive a request for data. This method
 * just queues up requests to be handled in the tick() method
*/
bool Cache::recvReq(Packet * packet){
	packet->ready_time = currCycle+accessDelay;
		
	DPRINTPACKET(label.c_str(),"Received Request", packet);

	if (pendingPackets.size() < reqQueueCapacity){
		pendingPackets.push_back(packet);
		return true;
	}
	DPRINTF(DEBUG_CACHE,
			"Packet - Cache:  Cache request queue full\n");
	return false;
}

bool Cache::processCacheMiss(Packet* packet){
	// First we need to get the block address (e.g., offset of 0)
	Location loc = getLocation(packet->addr);
	loc.offset = 0;
	uint32_t blockAddress = getAddress(loc);

	// Get a mshr entry - if no match, then just an empty one
	auto mshrMatchIt = std::find_if(mshr.begin(), mshr.end(),[blockAddress](MSHREntry entry){
		return entry.address == blockAddress && entry.valid;
	});
	if (mshrMatchIt == mshr.end()){
		// No direct match found, let's see if we can find an empty entry
		mshrMatchIt = std::find_if(mshr.begin(), mshr.end(),[](MSHREntry entry){
			return !entry.valid;
		});
		if (mshrMatchIt == mshr.end()){
			// No entries available.
			return false;
		}
	}
	auto entry = &(*mshrMatchIt);
	entry->address = blockAddress;

	// Find an open slot
	auto subEntries = &entry->subentries;
	auto subEntryIt = std::find_if(subEntries->begin(), subEntries->end(), [](MSHRSubEntry subEntry){
		return !subEntry.valid;
	});
	if (subEntryIt == subEntries->end()){
		return false;
	}

	// Let's add the subentry to MSHR
	auto subEntry = &(*subEntryIt);
	
	subEntry->packet = packet;

	// Now let's send a packet onward if we haven't sent it already
	if (!entry->issued){
		auto copiedPacket = repeatPacket(packet);
		if (copiedPacket->isWrite){
			// We just want to retrieve the data, we don't forward the write
			copiedPacket->isWrite = false;
			// Change type essentially we do not want it going back to L1_I cache
			copiedPacket->type = PacketTypeLoad;
		}
		auto response = next->recvReq(copiedPacket);
		if (!response){
			// Could not send packet on
			return false;
		}
		entry->issued = true;
	}

	// Let's wait to make this valid in case the forwarding failed
	subEntry->valid = true;
	entry->valid = true;
	return true;
}

void Cache::applyPacketToCacheBlock(Packet* packet, Block* block){
	Location loc = getLocation(packet->addr);
	if (packet->isWrite){
		// Write the data
		overwriteData(
				block->getData()+loc.offset,
				packet->data,
				packet->size);
		delete packet->data;
		packet->data = nullptr;
		block->setDirty(true);
		if(DEBUG_CACHE){
			DPRINTF(DEBUG_CACHE, "Block contents after write: ");
			auto data = block->getData();
			int count = getBlockSize()>16?16:getBlockSize();
			if (getBlockSize()>count) printf("... ");
			for(auto i = 0; i<count; i++){
				printf("%02X ",data[count-i-1]);
			}
			printf("\n");
		}
	}else{
		// Read the data
		overwriteData(
				packet->data,
				block->getData()+loc.offset,
				packet->size);						
	}
	packet->isReq = false;
}

bool Cache::writeBack(Block* block, uint32_t set){
	if (block->getValid() && block->getDirty()){
		Location loc;
		loc.set = set;
		loc.tag = block->getTag();
		loc.offset = 0;
		Packet* writeBack = new Packet(
			true,
			true,
			PacketTypeWriteBack,
			getAddress(loc),
			getBlockSize(),
			duplicateData(block->getData(),getBlockSize()),
			currCycle
			);
		DPRINTPACKET(label.c_str(),"Sending writeback", writeBack);
		if (!next->recvReq(writeBack)){
			assert(false && "Next cache stage cannot accomodate req");
		}
		return true;
	}
	return false;
}


/*
 * This function should be called when this cache is to receive a response to a request. It
 * services response immediately, does not queue for tick(). In addition to servicing read data,
 * this will send messages to mem pipe stage to let it know that operations have completed.
*/
void Cache::recvResp(Packet* packet){
	DPRINTPACKET(label.c_str(),"Received Response", packet);

	// Write packets don't return any data, if it's not a read response packet, then it will have data
	// that should be applied to the cache
	if (!packet->isWrite){
		// Apply packet
		Location loc = getLocation(packet->addr);
		Block* block;

		// Is it in cache? If not, we need to evict a cache line
		int way = getWay(packet->addr);
		
		// Is the block already in the cache?
		if (way == -1){
			// Allocate a new way
			Block* victimBlock = replPolicy->getVictim(packet->addr,packet->isWrite);
			// Do we need to do writeback?
			writeBack(victimBlock, loc.set);

			// Clear
			// No need to empty out blocks data because it will all be overwritten b/c block size
			// is the same throughout the cache levels
			victimBlock->clear(loc.tag);

			block = victimBlock;
		}else{
 			block = blocks[loc.set][way];
		}
		
		// Apply the new data received from higher cache levels
		overwriteData(block->getData(), packet->data, getBlockSize());
		DPRINTPACKET(label.c_str(),"Writing to packet", packet);

		// Now, was there a pending packets in the MSHR that needs to be serviced?
		auto mshrEntryIt = std::find_if(mshr.begin(), mshr.end(),[&packet](MSHREntry entry){
			return entry.address = packet->addr;
		});
		auto entry = &(*mshrEntryIt);

		// If we found it AND it is valid
		if (mshrEntryIt != mshr.end() && (*mshrEntryIt).valid){
			// Go through each valid subentry and deal with it
			auto subEntries = &(*mshrEntryIt).subentries;

			auto subEntriesToProcessEnd = std::partition(subEntries->begin(), subEntries->end(),[](MSHRSubEntry sub){return sub.valid;});

			std::transform(subEntries->begin(), subEntriesToProcessEnd, subEntries->begin(), [block, this](MSHRSubEntry subEntry){

				DPRINTPACKET(label.c_str(),"Applying MSHR subEntry packet", subEntry.packet);
				
				// Deal with waiting packet
				applyPacketToCacheBlock(subEntry.packet,block);

				DPRINTPACKET(label.c_str(),"Sending Response back", subEntry.packet);

				// Send response for waiting packet
				getPrev(subEntry.packet)->recvResp(subEntry.packet);
				subEntry.packet = nullptr;
				subEntry.valid = false;
				return subEntry;
			});

			entry->valid = false;
			entry->issued = false;
			entry->address = 0;
			// We are done with the packet - we can delete it
			DPRINTPACKET(label.c_str(), "Deleting packet: ", packet);
			delete packet;
		}else{
			assert(false && "no MSHR entry found");
		}
	}else{
		assert(false && "Received write packet from higher memory");
	}
}

/**
 * This will create a copy of a packet but with the correct block size and starting address,
 * suitable for repeating a message onwards
*/
Packet* Cache::repeatPacket(Packet* request){
	uint32_t blockAddress = (request->addr/getBlockSize())*getBlockSize();
	uint8_t* data = new uint8_t[getBlockSize()];

	auto duplicatedPacket = new Packet(request->isReq, request->isWrite, request->type, blockAddress,
			getBlockSize(), data, request->ready_time, request->originalPacket?request->originalPacket:request);

	return duplicatedPacket;
}

/*Processes packets*/
void Cache::Tick(){
	auto readyLambda = [this](Packet*packet){
		return packet->ready_time<=currCycle;
	};

	// Push the ready packets to the beginning of the vector and returns the iterator at the dividing line
	auto readyEnd = std::partition(pendingPackets.begin(),pendingPackets.end(),readyLambda);


	// Process each ready packet
	std::for_each(pendingPackets.begin(), readyEnd,[this](Packet* packet){
		DPRINTPACKET(label.c_str(),"Processing packet", packet);
		auto way = getWay(packet->addr);
		// If the block is not yet in cache, we need to get it and have the packet wait as pending
		if (way == -1){
			// Cache miss
			auto result = processCacheMiss(packet);
			if (!result){
				// Couldn't add to MSHR? Just say it will be ready next time
				// This DOES NOT STALL the pipeline unless the pendingPackets queue gets full
				// because pendingPackets queue is not part of the spec I do not believe this is the correct way
				// to go. Maybe we should be checking for cache hit and MSHR space allocation at that point
				// but I'm not sure
				packet->ready_time = currCycle + 1;
			}
		}else{
			// Cache hit
			DPRINTPACKET(label.c_str(),"Cache hit, apply packet", packet);
			Location loc = getLocation(packet->addr);
			applyPacketToCacheBlock(packet, blocks[loc.set][way]);
			if (packet->type == PacketTypeWriteBack){
				delete packet;
			}else{
				getPrev(packet)->recvResp(packet);
			}			
		};
	});

	// Remove the packets that were serviced. We must recalculate the range (remove_if) because some packets may have
	// been added to pending packets during the foreach above. Pendingpackets will have changed - requiring a second
	// call to .begin and .end
	pendingPackets.erase(std::remove_if(pendingPackets.begin(),pendingPackets.end(),readyLambda),pendingPackets.end());
}

/**
 * This gets applicable data to the range. If the data is not within
 * the appropriate range, it ignores it
*/
void Cache::dumpRead(uint32_t addr, uint32_t size, uint8_t* data){
	for (int i = 0; i<size;){
		Location loc = getLocation(addr+i);
		// If we are going to our next block, let's get the entire block,
		// not just from the offset
		if (i) loc.offset = 0;
		
		uint8_t way = getWay(addr);
		uint8_t* cacheData = 0;
		if (way != (uint8_t)-1){
			cacheData = blocks[loc.set][way]->getData();
		}
		for (;i<size && getLocation(addr+i).set == loc.set; i++){
			if (cacheData) *(data + i) = cacheData[i+loc.offset];
		}
	}
}


/*
 * L2 Cache only differs in the getPrev() function (and the prevInst/prevData values)
*/
BaseObject* L2Cache::getPrev(Packet* pkt){
	switch (pkt->type)
	{
		case PacketTypeFetch:
		case PacketTypePrefetch:
			return prevL1Inst;
	}
	return prevL1Data;
}