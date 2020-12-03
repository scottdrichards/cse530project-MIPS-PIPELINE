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

	switch (replType) {
	case RandomReplPolicy:
		replPolicy = new RandomRepl(this);
		break;
	default:
		assert(false && "Unknown Replacement Policy");
	}

	/*
	 * add other required initialization here
	 * (e.g: prefetcher and mshr)
	 */

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

int Cache::getWay(uint32_t addr) {
	uint32_t _addr = addr / blkSize;
	uint32_t setIndex = (numSets - 1) & _addr;
	uint32_t addrTag = _addr / numSets;
	for (int i = 0; i < (int) associativity; i++) {
		if ((blocks[setIndex][i]->getValid() == true)
				&& (blocks[setIndex][i]->getTag() == addrTag)) {
			return i;
		}
	}
	return -1;
}


/**
 * This function should be called when this cache is to receive a request for data
*/
bool Cache::sendReq(Packet * pkt){
	pkt->ready_time += accessDelay;
	if (reqQueue.size() < reqQueueCapacity){
		reqQueue.push(pkt);
		return true;
	}
	return false;
}

/*
 * This function should be called when this cache is to receive data in response to a request
*/
void Cache::recvResp(Packet* readRespPkt){
	// Find misses that are relevant to this data packet
	ReplacementPolicy *p = new ReplacementPolicy(this);

	prev->recvResp(readRespPkt);
	return;
}

/*You should complete this function*/
void Cache::Tick(){
	while (!reqQueue.empty()) {
		//check if any packet is ready to be serviced (we assume packets are in order
		// so if one is found that is not ready, none of the remainder are and we exit)
		if (reqQueue.front()->ready_time <= currCycle) {
			Packet* respPkt = reqQueue.front();
			reqQueue.pop();

			int tag = getWay(respPkt->addr);
			bool hit = tag != -1;

			if (hit){
				// Get the data
				uint32_t _addr = respPkt->addr / blkSize;
				uint32_t setIndex = (numSets - 1) & _addr;
				uint32_t offset = respPkt->addr & (blkSize-1);
				uint8_t* blockData = blocks[setIndex][tag]->getData();

				respPkt->data = &blockData[offset];
				respPkt->isReq = false;
				prev->recvResp(respPkt);
			}else{
				// Miss, send on to next cache
				next->sendReq(respPkt);
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
