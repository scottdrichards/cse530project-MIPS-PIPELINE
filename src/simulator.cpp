/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdio>
#include <iostream>
#include "simulator.h"
#include "util.h"

uint64_t currCycle;

Simulator::Simulator(ConfigurationData* info) {
	currCycle = 0;
	printf("initialize simulator\n\n");
	//initializing core
	pipe = new PipeState();

	// Initialize caches
	cache_l1_I = new L1ICache(
			info->cache_size_l1,
			info->cache_assoc_l1,
			info->cache_blk_size,
			(ReplacementPolicy) info->repl_policy_l1i,
			info->access_delay_l1);

	cache_l1_D = new L1DCache(
			info->cache_size_l1,
			info->cache_assoc_l1,
			info->cache_blk_size,
			(ReplacementPolicy) info->repl_policy_l1d,
			info->access_delay_l1);

	cache_l2 = new L2Cache(
			info->cache_size_l2,
			info->cache_assoc_l2,
			info->cache_blk_size,
			(ReplacementPolicy) info->repl_policy_l2,
			info->access_delay_l2);

	main_memory = new BaseMemory(info->memDelay);

	// Establish cache connections
	cache_l1_I->prev = pipe;
	cache_l1_I->next = cache_l2;

	cache_l1_D->prev = pipe;
	cache_l1_D->next = cache_l2;

	cache_l2->prevL1Data = cache_l1_D;
	cache_l2->prevL1Inst = cache_l1_I;
	cache_l2->next = main_memory;

	main_memory->prev = cache_l2;
	main_memory->next = nullptr;

	pipe->inst_mem = cache_l1_I;
	pipe->data_mem = cache_l1_D;

	// Create branch predictor
	switch (info->branch_predictor){
		default:
			pipe->BP = new DynamicBranchPredictor(
				info->bht_entries,
				info->bht_entry_width,
				info->pht_width,
				info->btb_size,
				info->ras_size
			);
	}
}

void printCycleHeader(){
	int headerLen = 40;
	std::cout << TERM_INVERSE;
	std::cout << std::string(headerLen, ' ') << "\n";
	std::cout << std::string(headerLen, ' ') << '\r';
	std::cout << std::string(headerLen/2 -5, ' ') << "Cycle: "<<currCycle<<"\n";
	std::cout << std::string(headerLen, ' ') << "\n";
	std::cout << TERM_RESET;
}

void Simulator::cycle() {
	if (DEBUG_CACHE||DEBUG_MEMORY||DEBUG_PIPE) printCycleHeader();

	// Check memory and caches for updates
	main_memory->Tick();
	cache_l2->Tick();
	cache_l1_I->Tick();
	cache_l1_D->Tick();
	//progress of the pipeline in this clock
	pipe->pipeCycle();
	pipe->stat_cycles++;
	// increment the global clock of the simulator
	currCycle++;
	
	if(DEBUG_CACHE||DEBUG_MEMORY||DEBUG_PIPE) std::cout<<"\n";
}



void Simulator::run(int num_cycles) {
	int i;
	if (pipe->RUN_BIT == false) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating for %d cycles...\n\n", num_cycles);
	for (i = 0; i < num_cycles; i++) {
		if (pipe->RUN_BIT == false) {
			printf("Simulator halted\n\n");
			break;
		}
		cycle();
	}
}


void Simulator::go() {
	if (pipe->RUN_BIT == false) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating...\n\n");
	while (pipe->RUN_BIT)
		cycle();
	printf("Simulator halted\n\n");
}



uint32_t Simulator::readMemForDump(uint32_t address) {
	uint32_t data;
	main_memory->dumpRead(address, 4, (uint8_t*) &data);
	cache_l2->dumpRead(address, 4, (uint8_t*) &data);
	cache_l1_I->dumpRead(address, 4, (uint8_t*) &data);
	cache_l1_D->dumpRead(address, 4, (uint8_t*) &data);
	return data;
}

void Simulator::registerDump() {
	int i;

	printf("PC: 0x%08x\n", pipe->PC);

	for (i = 0; i < 32; i++) {
		printf("R%d: 0x%08x\n", i, pipe->REGS[i]);
	}

	printf("HI: 0x%08x\n", pipe->HI);
	printf("LO: 0x%08x\n", pipe->LO);
	printf("Cycles: %u\n", pipe->stat_cycles);
	printf("FetchedInstr: %u\n", pipe->stat_inst_fetch);
	printf("RetiredInstr: %u\n", pipe->stat_inst_retire);
	printf("IPC: %0.3f\n",
			((float) pipe->stat_inst_retire) / pipe->stat_cycles);
	printf("Flushes: %u\n", pipe->stat_squash);
}

void Simulator::memDump(int start, int stop) {
	int address;

	printf("\nMemory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------\n");
	for (address = start; address < stop; address += 4) {
		printf("MEM[0x%08x]: 0x%08x\n", address, readMemForDump(address));
	}
	printf("\n");
}

Simulator::~Simulator() {
	delete main_memory;
	delete pipe;
}
