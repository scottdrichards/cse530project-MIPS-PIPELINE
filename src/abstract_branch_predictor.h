/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __ABSTRACT_BRANCH_PREDICTOR_H__
#define __ABSTRACT_BRANCH_PREDICTOR_H__
#include <cstdint>

/*
 * AbstractBranchPredictor should be Inherited by your
 * branch predictor
 */
class AbstractBranchPredictor {
public:
	AbstractBranchPredictor();
	virtual ~AbstractBranchPredictor();
	/*
	 * If it predicts that this PC belongs to a branch
	 * or jump, returns the predicted target. O.w. should
	 * return -1 as it is predicted no branch
	 */
	virtual uint32_t getTarget(uint32_t PC) = 0;

	virtual bool predictTaken(uint32_t PC){return false;};
	
	virtual uint32_t popRAS()=0;
	/**
	 * @returnPC is the return PC. So typically +4 of current PC when called
	*/
	virtual void pushRAS(uint32_t returnPC)=0;
	
	/*
	 * It is called after each branch is resolved to update
	 * the branch predictor metadata
	 */
	virtual void update(uint32_t PC, bool taken, uint32_t target) = 0;
};

#endif
