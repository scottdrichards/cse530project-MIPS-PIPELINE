/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __REPL_POLICY_H__
#define __REPL_POLICY_H__

#include <vector>
#include <cstdint>

class Cache;
class Block;


enum ReplacementPolicy{
	RandomReplPolicy,
	LRUReplPolicy,
	PLRUReplPolicy
};

/*
 * AbstarctReplacementPolicy should be inherited by your
 * implemented replacement policy
 */
class AbstarctReplacementPolicy {
public:
	AbstarctReplacementPolicy(Cache* cache);
	virtual ~AbstarctReplacementPolicy() {}
	//pointer to the Cache
	Cache* cache;
	/*
	 * should return the victim block based on the replacement
	 * policy- the caller should invalidate this block
	 */
	virtual Block* getVictim(uint32_t addr, bool isWrite) = 0;
	/*
	 * Called for both hit and miss.
	 * Should update the replacement policy metadata.
	 */
	virtual void update(uint32_t addr, int way, bool isWrite) = 0;
};

/*
 * Random replacement policy
 */
class RandomRepl: public AbstarctReplacementPolicy {
public:
	RandomRepl(Cache* cache);
	~RandomRepl() {}
	virtual Block* getVictim(uint32_t addr, bool isWrite) override;
	virtual void update(uint32_t addr, int way, bool isWrite) override;
};

/*
 * Least Recently Used (LRU) replacement policy 
*/
class LRURepl: public AbstarctReplacementPolicy{
	private:
		// Keeps track of the ordering of way indices
		typedef std::vector<uint8_t> Ordering;
		std::vector<Ordering> useTracker;
	public:
		LRURepl(Cache* cache);
		~LRURepl(){}
		virtual Block* getVictim(uint32_t addr, bool isWrite) override;
		virtual void update(uint32_t addr, int way, bool isWrite) override;
};

/*
 * Psuedo-Least Recently Used (PLRU) replacement policy 
 * https://en.wikipedia.org/wiki/Pseudo-LRU
*/
class PLRURepl: public AbstarctReplacementPolicy{
	private:
		// TreeArray is a list of bools
		typedef std::vector<bool> TreeArray;
		std::vector<TreeArray> useTracker;
	public:
		PLRURepl(Cache* cache);
		~PLRURepl(){}
		virtual Block* getVictim(uint32_t addr, bool isWrite) override;
		virtual void update(uint32_t addr, int way, bool isWrite) override;
};

#endif
