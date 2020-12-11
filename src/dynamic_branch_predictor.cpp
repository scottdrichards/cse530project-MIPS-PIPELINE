/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "dynamic_branch_predictor.h"
#include <cstdio>
#include <algorithm>    // std::find_if

DynamicBranchPredictor::DynamicBranchPredictor(
		uint32_t bht_entries,
		uint32_t bht_entry_width,
		uint32_t pht_width,
		uint32_t btb_size,
		uint32_t ras_size)
{
	this->bht_entries = bht_entries;
	this->bht_entry_width = bht_entry_width;
	this->pht_width = pht_width;
	this->btb_size = btb_size;
	this->ras_size = ras_size;

	BTB = std::vector<BTBEntry>(btb_size,{0,0,false});

}

DynamicBranchPredictor::~DynamicBranchPredictor() {

}

/**
 * Used for BTB - occurs at fetch to see what next instruction might be 
 */
uint32_t DynamicBranchPredictor::getTarget(uint32_t PC) {
	auto found = std::find_if(BTB.begin(), BTB.end(),
				[PC](BTBEntry entry){return entry.PC == PC && entry.valid;});
	
	if (found != BTB.end()) return (*found).target;
	return -1;
}


bool DynamicBranchPredictor::predictTaken(uint32_t PC){
	return false;
}


/**
 *  This is called whenever a branch has occurred
 */
void DynamicBranchPredictor::update(uint32_t PC, bool taken, uint32_t target) {
	/**
	 * BTB Section
	*/

	// Get existing entry
	auto found = std::find_if(BTB.begin(), BTB.end(),
			[PC](BTBEntry entry){return entry.PC == PC && entry.valid;});
	if (taken){
		// If no match, see if there's an empty entry
		if (found == BTB.end()) found = std::find_if(BTB.begin(), BTB.end(),
					[PC](BTBEntry entry){return entry.PC == PC && entry.valid;});
		
		// Update if we have a usable entry
		if (found != BTB.end()){
			auto entry = *found;
			entry.PC = PC;
			entry.target = target;
			entry.valid = true;
		}
	}else{
		if (found != BTB.end()){
			// Clear the entry
			(*found).valid = false;
		}
	}


	/**
	 * BHT Section
	*/

	return;
}
