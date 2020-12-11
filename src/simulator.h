/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "base_memory.h"
#include "pipe.h"
#include "util.h"
#include "cache.h"

class Simulator {
public:
	Simulator(ConfigurationData* info);
	virtual ~Simulator();
	PipeState * pipe;
	BaseMemory * main_memory;
	Cache * cache_l1_I;
	Cache * cache_l1_D;
	L2Cache * cache_l2;

	/*
	 * Execute a cycle
	 */
	void cycle();

	/*
	 * Simulation for n cycles
	 */
	void run(int num_cycles);

	/*
	 * Simulation until HALTed
	 */
	void go();

	// Debug functions

	/*
	 * Read a 32-bit word from memory for memDump
	 */
	uint32_t readMemForDump(uint32_t address);

	/*
	 * Print architectural registers and other stats
	 */
	void registerDump();

	/*
	 * Print a word-aligned region of memory to the output file
	 */
	void memDump(int start, int stop);


};

#endif
