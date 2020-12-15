			/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __UTIL_H__
#define __UTIL_H__
#include <cstdint>
#include "repl_policy.h"
#define mshr_size 8
#define mshr_subsets 2
#define mshr_columns 3+3*mshr_subsets

uint64_t extern currCycle;

extern bool DEBUG_MEMORY;
extern bool DEBUG_PIPE;
extern bool DEBUG_CACHE;
extern bool DEBUG_PREFETCH;

extern bool TRACE_MEMORY;

#define DPRINTF(flag, fmt, ...) \
	if(flag) \
        fprintf(stdout, "Cycle %5lu : [%-20s][%-20s]%4d: " fmt, currCycle, __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#define DPRINTPACKET(src, message, packet)\
	DPRINTF(DEBUG_CACHE, "%8s: %-25s %s\n",src,message,packet->toString().c_str())

#define TRACE(flag, cond, fmt, ...) \
	if((flag) && (cond)) \
        fprintf(stdout, fmt, ##__VA_ARGS__);

enum PacketSrcType {
	PacketTypeFetch = 0,
	PacketTypeLoad = 1,
	PacketTypeStore = 2,
	PacketTypePrefetch = 3,
	PacketTypeWriteBack = 4
};

class ConfigurationData{
public:
	// Cache
	uint64_t cache_blk_size;
	bool writeBack;
	
	uint64_t          cache_size_l1;
	uint64_t          cache_assoc_l1;
	uint32_t          cache_l1_mshr_entries;
	uint32_t          cache_l1_mshr_subentries;
	uint32_t          cache_l1_wbb_entries;
	uint64_t          access_delay_l1;
	ReplacementPolicy repl_policy_l1i;
	ReplacementPolicy repl_policy_l1d;
	
	uint64_t          cache_size_l2;
	uint64_t          cache_assoc_l2;
	uint32_t          cache_l2_mshr_entries;
	uint32_t          cache_l2_mshr_subentries;
	uint32_t          cache_l2_wbb_entries;
	uint32_t          access_delay_l2;
	ReplacementPolicy repl_policy_l2;

	uint32_t memDelay;


	// Branching
	uint32_t bht_entries;
	uint32_t bht_entry_width;
	uint32_t pht_width;
	uint32_t btb_size;
	uint32_t ras_size;
	int branch_predictor; // TODO - what should this be

	ConfigurationData() {
		cache_blk_size = 64;
		writeBack = true;

		cache_size_l1 = 32768;
		cache_assoc_l1 = 4;
		cache_l1_mshr_entries = 8;
		cache_l1_mshr_subentries = 2;
		cache_l1_wbb_entries = 8;
		access_delay_l1 = 2;
		repl_policy_l1i = ReplacementPolicy::RandomReplPolicy;
		repl_policy_l1d = ReplacementPolicy::RandomReplPolicy;

		cache_size_l2 = 2 * 1024 * 1024;
		cache_assoc_l2 = 16;
		repl_policy_l2 = ReplacementPolicy::RandomReplPolicy;
		access_delay_l2 = 20;
		cache_l2_mshr_entries = 32;
		cache_l2_mshr_subentries = 8;
		cache_l2_wbb_entries = 32;
		memDelay = 100;

		bht_entries = 2048;
		bht_entry_width = 8;
		pht_width = 2;
		btb_size = 1024;
		branch_predictor = 2;
		ras_size = 16;
	}
};

/**
 * Terminal codes
*/
#define TERM_CODE_START "\033["
#define TERM_CODE_END "m"
#define TERM_CODE(x) TERM_CODE_START x TERM_CODE_END
#define TERM_INVERSE TERM_CODE("7")
#define TERM_RESET TERM_CODE("0")

#endif
