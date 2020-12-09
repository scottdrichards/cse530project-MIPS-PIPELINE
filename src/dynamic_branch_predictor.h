/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_DYNAMIC_BRANCH_PREDICTOR_H_
#define SRC_DYNAMIC_BRANCH_PREDICTOR_H_

#include "abstract_branch_predictor.h"
/*
 * Dynamic branch predictor
 */
class DynamicBranchPredictor: public AbstractBranchPredictor {
public:
	DynamicBranchPredictor();
	virtual ~DynamicBranchPredictor();
	virtual uint32_t getTarget(uint32_t PC) override;
	virtual void update(uint32_t PC, bool taken, uint32_t target) override;
};

#endif /* SRC_DYNAMIC_BRANCH_PREDICTOR_H_ */
