/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef BASE_OBJECT_H_
#define BASE_OBJECT_H_
#include <stdio.h>
#include "util.h"

/*
 * Packet is used for communicating the memory requests between
 * core/prefetcher and caches/memory
 */
class Packet {
public:

	Packet(bool _isReq, bool _isWrite, PacketSrcType _type, uint32_t _addr,
			uint32_t _size, uint8_t* _data, uint32_t _ready_time) {
		isReq = _isReq;
		isWrite = _isWrite;
		type = _type;
		addr = _addr;
		size = _size;
		data = _data;
		ready_time = _ready_time;
	}
	virtual ~Packet() {
		if (data != nullptr)
			delete data;
	}
	//is this a request packet?
	bool isReq;
	//is this a write packet?
	bool isWrite;
	//what is the source of generating this packet?
	PacketSrcType type;
	uint32_t addr;
	uint32_t size;
	uint8_t* data;
	//when should this packet be serviced?
	uint32_t ready_time;
	void print(){
		printf("[Packet] addr: 0x%x, ready at: %4d, type: %2d, size: %2d, write: %d",
			addr, ready_time, type, size, isWrite);
		if ((isWrite && isReq) || (!isWrite && !isReq)){
			printf(" | Data (hex): ");
			for (uint32_t i = 0; i<size && i<16; i++){
				printf("%02X ",data[i]);
			}
			if (size>16) printf("...");
		}
		printf("\n");
	}
};

/*
 * BaseObject should be inherited by AbstractMemory and Pipeline
 */
class BaseObject {
public:
	BaseObject();
	virtual ~BaseObject();

	//send a request to this base object
	virtual bool recvReq(Packet * pkt) = 0;
	//this base object has received a packet
	virtual void recvResp(Packet* readRespPkt) = 0;
};

#endif /* BASE_OBJECT_H_ */
