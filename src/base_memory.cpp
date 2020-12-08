/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "base_memory.h"
#include "util.h"

BaseMemory::BaseMemory(uint32_t memDelay) :
		AbstractMemory(memDelay, 100) {
	//memory will be dynamically allocated at initialization
	MEM_REGIONS[0] = {MEM_TEXT_START, MEM_TEXT_SIZE, nullptr};
	MEM_REGIONS[1] = {MEM_DATA_START, MEM_DATA_SIZE, nullptr};
	MEM_REGIONS[2] = {MEM_STACK_START, MEM_STACK_SIZE, nullptr};
	MEM_REGIONS[3] = {MEM_KDATA_START, MEM_KDATA_SIZE, nullptr};
	MEM_REGIONS[4] = {MEM_KTEXT_START, MEM_KTEXT_SIZE, nullptr};

	for (int i = 0; i < MEM_NREGIONS; i++) {
		MEM_REGIONS[i].mem = (uint8_t *)malloc(MEM_REGIONS[i].size);
		memset(MEM_REGIONS[i].mem, 0, MEM_REGIONS[i].size);
	}
}

BaseMemory::~BaseMemory() {
	for (int i = 0; i < MEM_NREGIONS; i++)
		free(MEM_REGIONS[i].mem);
}

bool BaseMemory::recvReq(Packet * pkt) {

	TRACE(TRACE_MEMORY, pkt->type == PacketTypeStore,
			"mdump 0x%x 0x%x\n", pkt->addr, pkt->addr + pkt->size);

	mem_region_t* mem_region = getMemRegion(pkt->addr, pkt->size);

	//if the accessed memory region is valid
	if (mem_region) {
		//update the time to service the packet
		pkt->ready_time += accessDelay;
		
		DPRINTPACKET("Memory","Receive", pkt);
		
		//add the packet to request queue if it has free entry
		if (pendingPackets.size() < reqQueueCapacity) {
			pendingPackets.push_back(pkt);
			//return true since memory received the request successfully
			return true;
		} else {
			/*
			 * return false since memory could not add the packet
			 * to request queue, the source of packet should retry
			 */
			return false;
		}

	} else {
		//access to invalid region of memory
		for (int i = 0; i < MEM_NREGIONS; i++) {
			std::cerr << "MemoryRegion #" << i << " : " << std::hex
					<< MEM_REGIONS[i].start << " "
					<< MEM_REGIONS[i].start + MEM_REGIONS[i].size << std::dec
					<< "\n";
		}
		std::cerr << "Access to a unallocated region of memory : addr : "
				<< std::hex << pkt->addr << " " << pkt->addr + pkt->size
				<< std::dec << "\n";
		assert(false);
	}
}

void BaseMemory::recvResp(Packet* pkt) {
	assert(false && "No one send respond to memory");
	return;
}

mem_region_t*
BaseMemory::getMemRegion(uint32_t addr, uint32_t size) {
	for (int i = 0; i < MEM_NREGIONS; i++) {
		if (addr >= MEM_REGIONS[i].start
				&& (addr + size)
						< (MEM_REGIONS[i].start + MEM_REGIONS[i].size)) {
			return &MEM_REGIONS[i];
		}
	}
	return nullptr;
}

void BaseMemory::Tick() {
	while (!pendingPackets.empty()) {
		//check if any packet is ready to be serviced
		if (pendingPackets.front()->ready_time <= currCycle) {
			Packet* respPkt = pendingPackets.front();
			pendingPackets.erase(pendingPackets.begin());
			
			DPRINTPACKET("Memory","Send", respPkt);
			
			if (respPkt->isWrite) {
				mem_region_t* mem_region = getMemRegion(respPkt->addr,
						respPkt->size);
				int index = respPkt->addr - mem_region->start;
				//perform the write in the memory
				for (uint32_t i = 0; i < respPkt->size; i++) {
					mem_region->mem[index + i] = *(respPkt->data + i);
				}
				//change this pkt to respond pkt
				respPkt->isReq = false;
				//the data part is no longer needed
				delete respPkt->data;
				respPkt->data = nullptr;
				/*
				 * send the respond to the previous base_object which is waiting
				 * for this respond packet. For now, prev for memory is core but
				 * you should update the prev since you are adding the caches
				 */
				if (respPkt->type == PacketTypeWriteBack){
					delete respPkt;
				}
				else{
					prev->recvResp(respPkt);
				}
			} else {
				mem_region_t* mem_region = getMemRegion(respPkt->addr,
						respPkt->size);
				int index = respPkt->addr - mem_region->start;
				//perform the read
				for (uint32_t i = 0; i < respPkt->size; i++) {
					*(respPkt->data + i) = mem_region->mem[index + i];
				}
				respPkt->isReq = false;
				/*
				 * send the respond to the previous base_object which is waiting
				 * for this respond packet. For now, prev for memory is core but
				 * you should update the prev since you are adding the caches
				 */
				prev->recvResp(respPkt);
			}
		} else {
			/*
			 * assume that pendingRequests is sorted by ready_time for now
			 * (because the pipeline is in-order)
			 */
			break;
		}
	}
	return;
}

/*
 * you should use this function and implement a similar
 * functionality for this function in the Cache. This read
 * operation is only for debug so avoid using mshr for this
 * function
 */
void BaseMemory::dumpRead(uint32_t addr, uint32_t size, uint8_t* data) {
	mem_region_t* mem_region = getMemRegion(addr, size);
	int index = addr - mem_region->start;
	for (uint32_t i = 0; i < size; i++) {
		*(data + i) = mem_region->mem[index + i];
	}
}
