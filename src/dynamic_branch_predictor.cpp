/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "dynamic_branch_predictor.h"
#include <cstdio>
//2^k entries, where k = 5 
//columns: Entry PC(27 tag=5) | Predicted PC | Valid
uint32_t BTB[32][3]={{0},{0},{0}};

DynamicBranchPredictor::DynamicBranchPredictor() {
	
}

DynamicBranchPredictor::~DynamicBranchPredictor() {

}

/*
 * getTarget : at the end of Fetch stage call
 * 
 */
uint32_t DynamicBranchPredictor::getTarget(uint32_t PC) {
	//scan BTB to see if there's an entry found
	for (int i=0; i<32; i++){
			//if the entry in the BTB is the same as the PC			
			if(BTB[i][0]==PC){
			// Send out predicted PC to Decode stage
			//PC=BTB[i][1];
			//return PC + 4; //BTB[i][1](target): PC + 4
			}
	}
	//else just return PC+4 Adder
	return PC + 4;
}

/*
 * update : at the end of Execute stage call
 * 
 */
void DynamicBranchPredictor::update(uint32_t PC, bool taken, uint32_t target) {
	bool duplicate = false;
	//scan BTB in case an entry matches
	for (int i=0; i<32; i++){
			//PC and target are the same, don't re-enter this entry.			
			if(BTB[i][0]==PC && BTB[i][1]==target){
			printf("BTB entry already there.");
			duplicate = true;
			}
	}
	
	//if branch/jump is taken
	if (taken ==1 && duplicate == false){
		//Enter branch instr.add. and next PC into BTB
		for (int i=0; i<32; i++){
			//empty space found, so enter into the BTB
			if(BTB[i][0]==0){
			BTB[i][0]=PC; //tag
			BTB[i][1]=target; //target
			//some 2-bit branch predicton?
			//BTB[i][2]=prediction; valid?
			break;
			}
		}
	}//else no entry in BTB 
	
	return;
}
