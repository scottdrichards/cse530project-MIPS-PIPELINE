/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __CACHE_H__
#define __CACHE_H__

#include <cstdint>
#include <string>
#include <vector>

#include "block.h"
#include "abstract_memory.h"
#include "abstract_prefetcher.h"
#include "repl_policy.h"

// These are used so that we don't have to keep typing them all out for each constructor
#define CACHE_PARAMS_TYPED uint32_t size, uint32_t associativity, uint32_t blkSize,\
			ReplacementPolicy replacementPolicy, uint32_t delay, uint32_t mshr_entries, uint32_t mshr_subentries
#define CACHE_PARAMS size, associativity, blkSize, replacementPolicy, delay, mshr_entries, mshr_subentries

// This is a simple struct useful for splitting up addresses to tag/set/offset
// Not space efficient, but works for simulation
typedef struct Location{
	uint32_t tag;
	uint32_t set;
	uint32_t offset;
} Location;


class Cache: public AbstractMemory {
private:
	// MSHR Section
	struct MSHRSubEntry{
		bool valid;
		Packet* packet;
	};
	struct MSHREntry{
		bool valid;
		uint32_t address;
		bool issued;
		std::vector<MSHRSubEntry> subentries;
	};
	std::vector<MSHREntry> mshr;

	AbstarctReplacementPolicy *replPolicy;
	AbstractPrefetcher* prefetcher;
	uint64_t cSize, associativity, blkSize, numSets;

	Packet* repeatPacket(Packet* request);
	
	/**
	 * Returns a matching packet that is pending (e.g., a write or a read that was 
	 * translated to a cache block size)
	*/
	Packet* getStalledPacket(Packet* packet);
	/***
	 * This is used to apply a packet to a found and ready cache block
	 */
	void applyPacketToCacheBlock(Packet* packet, Block* block);
	/**
	 * Checks if writeback is required and performs it if so
	*/
	bool writeBack(Block* block, uint32_t set);

	/**
	 * Adds the packet to the MSHR
	*/
	bool processCacheMiss(Packet* packet);
public:
	// Used to calculate tag set and offset with an address
	Location getLocation(uint32_t addr);
	// Gets an address from tag, set, and offset
	uint32_t getAddress(Location location);
	// "l2Cache" "L1ICache" etc.
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
	/**
	 * This will give the current way holding the data, -1 means
	 * all ways are full
	*/
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
			label = "L1DCache";
		};
	virtual ~L1DCache(){};
};

#endif
