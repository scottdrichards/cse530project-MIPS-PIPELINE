/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef BASE_OBJECT_H_
#define BASE_OBJECT_H_
#include <stdio.h>
#include <sstream>
#include <string>
#include <iomanip>      // std::setfill, std::setw
#include "util.h"

/*
 * Packet is used for communicating the memory requests between
 * core/prefetcher and caches/memory
 */
class Packet {
public:
	Packet(bool _isReq, bool _isWrite, PacketSrcType _type, uint32_t _addr,
			uint32_t _size, uint8_t* _data, uint32_t _ready_time, Packet* _original_packet) {
		isReq = _isReq;
		isWrite = _isWrite;
		type = _type;
		addr = _addr;
		size = _size;
		data = _data;
		ready_time = _ready_time;
		originalPacket = _original_packet;
	}

	// Overloaded constructor that sets original packet as null
	Packet(bool _isReq, bool _isWrite, PacketSrcType _type, uint32_t _addr, uint32_t _size, uint8_t* _data, uint32_t _ready_time)
			:Packet(_isReq, _isWrite, _type, _addr, _size, _data, _ready_time, 0){}
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
	// Label is used to track the request and spawned requests in response to the original packet
	Packet* originalPacket;
	std::string toString(){
		std::stringstream ss;
		ss<<"[Packet] addr: 0x"<< std::uppercase << std::setfill('0') << std::setw(8) << std::hex << addr<< std::dec;
		ss<<", ready at: "<<std::setw(4)<<ready_time;
		ss<<", type: "<<std::setw(2);
		switch (type){
			case PacketTypeFetch:
				ss<<"FE";
				break;
			case PacketTypeLoad:
				ss<<"LD";
				break;
			case PacketTypeStore:
				ss<<"ST";
				break;
			case PacketTypePrefetch:
				ss<<"PF";
				break;
			case PacketTypeWriteBack:
				ss<<"WB";
				break;
		}
		ss<<", size: "<<std::setw(2)<<size;
		ss<<", write: "<<std::setw(1)<<isWrite;
		ss<<", OG Pkt: "<< std::uppercase << std::setfill('0') << std::setw(8) << std::hex << originalPacket<< std::dec;
		if (isWrite == isReq){
			ss<<" | Data (hex): ";
			int count = size>16?16:size;
			if (size>16) ss<<"... ";
			for (uint32_t i = 0; i<count; i++){
				ss<< std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(data[count-i-1])<<" "<<std::dec;
			}
		}
		return ss.str();
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
