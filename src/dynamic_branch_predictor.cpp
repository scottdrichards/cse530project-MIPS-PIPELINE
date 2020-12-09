/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "dynamic_branch_predictor.h"
#include <cstdio>
//2^k entries, where k = 5 
//columns: Entry PC(27 tag=5) | Predicted PC | Valid
uint32_t BTB[32][3];

DynamicBranchPredictor::DynamicBranchPredictor(uint32_t PC) {
	
}

DynamicBranchPredictor::~DynamicBranchPredictor() {

}

uint32_t DynamicBranchPredictor::getTarget(uint32_t PC) {
	// Here if we find an entry
	//for(BTB[][]  ){
	//	if( tag == (last five bits)){
	//		//send out predicted PC to ID stage

	//	}
	//}

	//if No Is instr. a taken branch?

	
	//always return next PC
	return PC + 4;
}

void DynamicBranchPredictor::update(uint32_t PC, bool taken, uint32_t target) {
	//if branch/jump is taken
	//if (taken ==1){
		//Enter branch instr.add. and next PC into BTB
	//	for (BTB[][]  )

	//}//else no entry in BTB
	
	return;
}
