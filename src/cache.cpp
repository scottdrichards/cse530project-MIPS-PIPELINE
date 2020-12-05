/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include "repl_policy.h"
#include "next_line_prefetcher.h"
#include "cache.h"

Cache::Cache(uint32_t size, uint32_t associativity, uint32_t blkSize,
		enum ReplacementPolicy replType, uint32_t delay):
		AbstractMemory(delay, 100),cSize(size),
		associativity(associativity), blkSize(blkSize) {

	numSets = cSize / (blkSize * associativity);
	blocks = new Block**[numSets];

	for (int i = 0; i < (int) numSets; i++) {
		blocks[i] = new Block*[associativity];
		for (int j = 0; j < associativity; j++)
			blocks[i][j] = new Block(blkSize);
	}

	next == nullptr;
	prev == nullptr;

	switch (replType) {
		case RandomReplPolicy:
			replPolicy = new RandomRepl(this);
			break;
		case LRUReplPolicy:
			replPolicy = new LRURepl(this);
		default:
			assert(false && "Unknown Replacement Policy");
	}

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
 * This function should be called when this cache is to receive a request for data. This method
 * just queues up requests to be handled in the tick() method
*/
bool Cache::recvReq(Packet * packet){
	packet->ready_time += accessDelay;
		
	// Debug print
	DPRINTF(DEBUG_CACHE,"%8s recved req ",label.c_str());
	if (DEBUG_CACHE)packet->print();

	if (reqQueue.size() < reqQueueCapacity){
		reqQueue.push(packet);
		return true;
	}
	DPRINTF(DEBUG_CACHE,
			"Packet - Cache:  Cache request queue full\n");
	return false;
}

/*
 * This function should be called when this cache is to receive a response to a request. It
 * services response immediately (though I'm not 100% confident on this),
 * does not queue for tick(). In addition to servicing read data,
 * this will send messages to mem pipe stage to let it know that operations have completed.
*/
void Cache::recvResp(Packet* packet){
	
	// Debug print
	DPRINTF(DEBUG_CACHE,"%8s recved rsp ",label.c_str());
	if (DEBUG_CACHE)packet->print();

	// First we need to see if we need to write data to this cache (i.e., was the original request
	// a read?)
	if (!packet->isWrite){
		Location loc = getLocation(packet->addr);
		int way = getWay(packet->addr);

		Block* block;
		if (way != -1){
			// Block is stored in cache (which would be weird... not sure why this would happen)
			block = blocks[loc.set][way];
			replPolicy->update(packet->addr,way,packet->isWrite);
		}else{
			// Block not in cache yet
			block = replPolicy->getVictim(packet->addr,packet->isWrite);
			if (block->getValid() && block->getDirty()){
				// Writeback
				
				uint32_t evictedAddress = getAddress({block->getTag(),loc.set,0});

				// We need to make a copy of the data for transit, it will get destroyed on receipt
				uint8_t* evictedData = block->getData();
				uint8_t* evictedDataCopy = new uint8_t[blkSize];
				for (uint32_t i = 0; i<getBlockSize();i++){
					evictedDataCopy[i] = evictedData[i];
				}

				Packet* writeBack = new Packet(false, true, PacketTypeWriteBack, evictedAddress,
													getBlockSize(),evictedDataCopy,0);
				
				
				// Debug print
				DPRINTF(DEBUG_CACHE,"%8s sending wb ",label.c_str());
				if (DEBUG_CACHE)writeBack->print();

				// TODO - buffer this message in case of it being full
				next->recvReq(writeBack);
			}
			// Clear block
			block->clear(loc.tag);
		}

		// Copy Data
		uint8_t* data = block->getData();
		for(uint32_t i = 0; i<getBlockSize();i++){
			data[i] = packet->data[i];
		}


		block->setValid(true);
		// There's no way a packet received here could make the cache dirty
		block->setDirty(false);

		DPRINTF(DEBUG_CACHE,"%s Writing response 0x%x => set %d tag %d offset %d\n",label.c_str(), packet->addr,loc.set,loc.tag, loc.offset);

		// Did this apply to a previous packet that is being held? Because it is not out of order, we don't need to service multiple
		// waiting packets in this section
		std::vector<Packet*>::iterator found = std::find_if(std::begin(holdingReqQueue),std::end(holdingReqQueue),
				[loc, this](Packet*originalReq){
			Location originalLoc = getLocation(originalReq->addr);
			return loc.set == originalLoc.set && loc.tag == originalLoc.tag;
		});

		if (found != std::end(holdingReqQueue)){
			delete packet;
			packet = *found;
			auto originalOffset =getLocation(packet->addr).offset;
			// Was the old one a write? we need to apply it
			if (packet->isWrite){
				DPRINTF(DEBUG_CACHE,"%8s applying staged write ",label.c_str());
				if (DEBUG_CACHE)packet->print();

				block->setDirty(true);
				// write the data to cache
				for (uint32_t i = 0; i< packet->size; i++){
					data[originalOffset+i] = packet->data[i];
				}			
				
				// Communicate that the write is complete.
				packet->isReq = false;
				delete packet->data;
				packet->data = nullptr;
			}else{
				// Not write - we need to grab data for the read
				for (uint32_t i=0; i<packet->size; i++){
					packet->data[i] = data[i+originalOffset];
				}

			}
		}	
	}

	getPrev(packet)->recvResp(packet);
	return;
}

/**
 * This will create a copy of a packet but with the correct block size
*/
Packet* Cache::repeatPacket(Packet* request){
	uint32_t blockAddress = (request->addr/getBlockSize())*getBlockSize();
	uint8_t* data = new uint8_t[getBlockSize()];
	auto duplicatedPacket = new Packet(request->isReq, request->isWrite, request->type, blockAddress,
			getBlockSize(), data, request->ready_time);
	return duplicatedPacket;
}

/*Processes packets*/
void Cache::Tick(){
	while (!reqQueue.empty()) {
		//check if any request packet is ready to be serviced (we assume packets are in order
		// so if one is found that is not ready, none of the remainder are and we exit)
		if (reqQueue.front()->ready_time <= currCycle) {
			Packet* packet = reqQueue.front();

			// Debug print
			DPRINTF(DEBUG_CACHE,"%8s processing ",label.c_str());
			if (DEBUG_CACHE)packet->print();

			reqQueue.pop();
			int tag = getWay(packet->addr);
			bool hit = tag != -1;

			if (hit){
				// Get the address parameters
				Location loc = getLocation(packet->addr);
				DPRINTF(DEBUG_CACHE,"%s Cache hit 0x%x => set %d tag %d offset %d\n",label.c_str(), packet->addr,loc.set,loc.tag, loc.offset);
				// Get the data
				Block* block = blocks[loc.set][tag];
				uint8_t* blockData = block->getData();
				if (packet->isWrite){
					// Write
					for (uint32_t i = 0; i<packet->size;i++){
						blockData[loc.offset+i] = packet->data[i];
					}
					if (packet->type == PacketTypeWriteBack){
						// Just kill the writeback, no need to bounce back its status
						// TODO: Writethrough and other policies
						delete packet;
					}else{
						// Communicate that the write is complete.
						packet->isReq = false;
						delete packet->data;
						packet->data = nullptr;
						getPrev(packet)->recvResp(packet);
					}
				}else{
					// Otherwise get the blockdata and return
					packet->isReq = false;
					// Copy data
					for (uint32_t i = 0; i<packet->size; i++){
						packet->data[i] = blockData[i+loc.offset];
					}
					getPrev(packet)->recvResp(packet);
				}				
			}else{
				Location loc = getLocation(packet->addr);
				DPRINTF(DEBUG_CACHE,"%s Cache miss 0x%x => set %d tag %d\n",label.c_str(), packet->addr,loc.set,loc.tag);
				// Miss, send on to next cache

				// Do we need to translate a read to be the entire block?
				if (packet->isWrite){
					// If it is a write, we need to grab the right block
					Packet* packetToSend = repeatPacket(packet);
					holdingReqQueue.push_back(packet);
					packet = packetToSend;
					packet->isWrite = false;
				}
				else if (packet->size != getBlockSize()){
					Packet* packetToSend = repeatPacket(packet);
					holdingReqQueue.push_back(packet);
					packet = packetToSend;
				}
				
				//TODO: Buffer this in case next stage is full - not absolutely necessary as the stage can hold 100 messages
				// and I'm not sure we are going over 100 cycles for a mem hit (though multiple things can send mem reqs...)
				next->recvReq(packet);
			}
		} else {
			/*
			 * assume that reqQueue is sorted by ready_time for now
			 * (because the pipeline is in-order)
			 */
			break;
		}
	}
	return;
}

/*You should complete this function*/
void Cache::dumpRead(uint32_t addr, uint32_t size, uint8_t* data){

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