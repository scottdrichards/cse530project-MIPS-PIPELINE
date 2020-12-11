/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "dynamic_branch_predictor.h"
#include <cstdio>
#include <iostream>
//2^k entries, where k = 5 
//columns: Entry PC(27 tag=5) | Predicted PC | Valid
uint32_t BTB[32][3]={{0},{0},{0}};
bool entry_found=false;
int entry_num=0;

DynamicBranchPredictor::DynamicBranchPredictor() {
	
}

DynamicBranchPredictor::~DynamicBranchPredictor() {

}


//BTB Flow

/*
 * getTarget : at the end of FETCH stage call
 * 
 */
uint32_t DynamicBranchPredictor::getTarget(uint32_t PC) {
	// Entry found in BTB?
	for (int i=0; i<32; i++){
			//Yes
			if(BTB[i][0]==PC){
			entry_found=true; //for sendout
			entry_num=i; //for sendout
			break;
			}
	}
	return PC + 4; 
}

/*
 * sendout : at the end of DECODE stage call
 * 
 */
uint32_t DynamicBranchPredictor::sendout() {
	//Yes Entry found!
	if(entry_found==true){
		//send out predicted PC in Decode Stage
		//reset other bool.
		entry_found=false;
		return BTB[entry_num][1];
	}
	//if not found, just return with zero
	return 0;
}

void DynamicBranchPredictor::misprediction() {
	//clear BTB entry!
	BTB[entry_num][0]=0;
	BTB[entry_num][1]=0;
	BTB[entry_num][2]=0;
	return;
}

/*
 * update : at the end of Execute stage call
 * 
 */
void DynamicBranchPredictor::update(uint32_t PC, bool taken, uint32_t target) {
	bool duplicate = false;
	//scan BTB in case an entry matches
	//Unique value?
	for (int i=0; i<32; i++){
			//PC and target are the same, don't re-enter this entry.			
			if(BTB[i][0]==PC && BTB[i][1]==target){
			std::cout << "BTB entry already there.";
			duplicate = true;
			break;
			}
	}

	//Is the instruction a taken branch? 
	if (taken ==1 && duplicate == false) {
		//Enter branch instr.add. and next PC into BTB
		for (int i=0; i<32; i++){
			//empty space found, so enter into the BTB
			if(BTB[i][0]==0){
			BTB[i][0]=PC; //tag
			BTB[i][1]=target; //target

			//some 2-bit branch predicton:
			// 0 - Strongly Not Taken
			// 1 - Weakly Not Taken
			// 2 - Weakly Taken
			// 3 - Strongly Taken
			//BTB[i][2]=3; 

			break;
			}
		}
	}

	return;
}
