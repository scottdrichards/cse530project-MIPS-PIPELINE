/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __CACHE_H__
#define __CACHE_H__

#include "block.h"
#include "abstract_memory.h"
#include "abstract_prefetcher.h"
#include "repl_policy.h"
#include <cstdint>
#include <string>

// These are used so that we don't have to keep typing them all out for each constructor
#define CACHE_PARAMS_TYPED uint32_t _Size, uint32_t _associativity, uint32_t _blkSize,\
			enum ReplacementPolicy _replPolicy, uint32_t _delay
#define CACHE_PARAMS _Size, _associativity, _blkSize, _replPolicy, _delay

/*
 * You should implement MSHR
 */
class MSHR {
public:
	MSHR();
	virtual ~MSHR();
};

// This is a simple struct for use in getLocation method
typedef struct Location{
	uint32_t tag;
	uint32_t set;
	uint32_t offset;
} Location;

/*
 * You should implement Cache
 */
class Cache: public AbstractMemory {
private:
	AbstarctReplacementPolicy *replPolicy;
	AbstractPrefetcher* prefetcher;
	MSHR* mshr;
	uint64_t cSize, associativity, blkSize, numSets;
	// This is mainly for L1 cache that will receive a byte request
	// and will have to translate it to a block size, we will hold onto the
	// request in this queue for when the response happens.
	std::vector<Packet*>holdingReqQueue;
	Packet* repeatPacket(Packet* request);
	// Used to calculate tag setindex and offset with an address
	Location getLocation(uint32_t addr);
	// Gets an address from tag, set, and offset
	uint32_t getAddress(Location location);
public:
	std::string label;
	//Pointer to an array of block pointers
	Block ***blocks;
	Cache(CACHE_PARAMS_TYPED);
	virtual ~Cache();
	/*
	 * Most memory only has one prev, but L2 must choose between D and I cache,
	 * so we can overload the function here 
	*/
	virtual BaseObject* getPrev(Packet* readRespPkt){return prev;};
	virtual bool recvReq(Packet * pkt) override;
	virtual void recvResp(Packet* readRespPkt) override;
	virtual void Tick() override;
	int getWay(uint32_t addr);
	virtual uint32_t getAssociativity();
	virtual uint32_t getNumSets();
	virtual uint32_t getBlockSize();
	/*
	 * read the data if it is in the cache. If it is not, read from memory.
	 * this is not a normal read operation, this is for debug, do not use
	 * mshr for implementing this
	 */
	virtual void dumpRead(uint32_t addr, uint32_t size, uint8_t* data) override;
	//place other functions here if necessary
};

/*
 * L2Cache object differs from Cache mainly because it has a prevL1Data and prevL1Inst
 * which are intended to replace the .prev of the cache base class. getPrev() is also
 * different so that it can select between the two
*/
class L2Cache: public Cache {
public:
	BaseObject* prevL1Data;
	BaseObject* prevL1Inst;
	// Pass on constructor to base class of Cache
	L2Cache(CACHE_PARAMS_TYPED)
		:Cache(CACHE_PARAMS){
			label = "L2Cache";
			next = nullptr;
		};
	virtual ~L2Cache(){};
	
	BaseObject* getPrev(Packet* readRespPkt);
};

/**
 * The L1 cache objects are only different for debugging purposes (getType)
*/
class L1ICache: public Cache{
public:
	L1ICache(CACHE_PARAMS_TYPED)
		:Cache(CACHE_PARAMS){
			label = "L1ICache";
		};
	virtual ~L1ICache(){};
};
class L1DCache: public Cache{
public:
	L1DCache(CACHE_PARAMS_TYPED)
		:Cache(CACHE_PARAMS){
			label = "L1ICache";
		};
	virtual ~L1DCache(){};
};

#endif
