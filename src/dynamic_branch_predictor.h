/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_DYNAMIC_BRANCH_PREDICTOR_H_
#define SRC_DYNAMIC_BRANCH_PREDICTOR_H_
#include <vector>
#include "abstract_branch_predictor.h"
/*
 * Dynamic branch predictor
 */
class DynamicBranchPredictor: public AbstractBranchPredictor {
private:
	struct BTBEntry
	{
		uint32_t PC;
		uint32_t target;
		bool valid;
	};
	std::vector<BTBEntry> BTB;

	uint8_t RAS_next = 0;
	uint8_t RAS_count =0;
	std::vector<uint32_t> RAS;

	uint32_t bht_entries;
	uint32_t bht_entry_width;
	uint32_t pht_width;
	uint32_t btb_size;
	uint32_t ras_size;
public:
	DynamicBranchPredictor(
			uint32_t bht_entries,
			uint32_t bht_entry_width,
			uint32_t pht_width,
			uint32_t btb_size,
			uint32_t ras_size);
	virtual ~DynamicBranchPredictor();
	virtual uint32_t getTarget(uint32_t PC) override;
	virtual bool predictTaken(uint32_t PC) override;
	virtual void update(uint32_t PC, bool taken, uint32_t target) override;
	virtual uint32_t popRAS() override;
	/**
	 * @returnPC is the return PC. So typically +4 of current PC when called
	*/
	virtual void pushRAS(uint32_t returnPC) override;
};

#endif /* SRC_DYNAMIC_BRANCH_PREDICTOR_H_ */
