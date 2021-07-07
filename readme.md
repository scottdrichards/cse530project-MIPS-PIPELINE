# MIPS Pipeline Project
Overview
-------------
This project was to implement a MIPS pipeline simulator that ingests machine code and similates pipeline operations such as forwarding, caching, branch prediction, etc. The clock and baseline pipeline was provided but all pipeline optimizations were required. We got full marks for satisfying all requirements 

The best way to view my contributions is to peruse [my commits](https://github.com/scottdrichards/cse530project/commits?author=scottdrichards) or to watch a [tutorial](https://www.dropbox.com/s/pmot5gegrnkz9p2/zoom_1.mp4?dl=0) I made (in vain) for my 6 teammates to understand the code. I did ~30k lines of code and was essentially the only contributor for this project.

Not only am I proud of my implementation, but I also am proud of the debug tools/environment I set up.


## Project Specification

README
-------------
This document contains the documentation for the 1st project 'Simulator'. The goal of this project is to extend a simulator prototype, which is an essential tool for computer architecture researchers. As the name suggests, simulator is used for cloning the executional behaviour of a computer architectural design. Typically, we are focussing on a class of simulators called 'Cycle-Accurate Simulators', which simulates a microarchitecture on a cycle-by-cycle basis. It means that the entire execution strategy is based on the cycles and delays incorporated by certain operations. Please note that this project is runnable on Linux.
Broadly, in this project, you are supposed to **extend the Inorder MIPS pipeline**, add **Branch Predictor** functionality, and **implement a Cache Memory Interface**, which are required for the functioning of a Cycle-Accurate Simulator.

Brief Summary of Baseline Simulator
-------------------------------------------------
You have been given the baseline simulator which you should use as the baseline and build your design on top of that. The simulator has a five-stage pipeline (MIPS pipeline). For interacting with memory hierarchy the pipeline has to send/receive packets to/from memory. Memory can accept or reject any incoming requests as the number of requests that can be received by it is limited. Moreover, the requests to memory are not resolved immediately. The pipeline has to receive a response packet which shows the request has been resolved. When the memory receives a memory request, it will first update the time at which the request is going to be serviced based on the memory delay. In each cycle, a Tick() function for memory is called in order to respond to the pending requests if they are ready to be serviced and send the response to core for the serviced request.

Inorder MIPS Pipeline
--------------------------------
For the first task you should study the pipeline code and try to implement forwarding/ bypassing. **You have to implement complete forwarding scheme of this pipeline similar to the schemes discussed in the slides**.

Cache Memory Interface
----------------------------------
The given simulator supports complete interaction between core and memory. You have to design a cache interface and use that to expand the simulator to have separate L1 Instruction and L1 Data and shared L2 for instructions and data.
The Cache Interface which you are supposed to implement follows the **inclusive policy** (all blocks in higher level cache are also in lower level cache). The cache interface is designed for set-associative caches.
**You are supposed to implement two configs for cache policy and compare their performance: 1) write-allocate + write-back policy and 2) write-allocate + write-through **. Inorder for the cache to support the write-back policy you have to implement write-back buffer. Using this structure, when a dirty block is evicted from the cache, it would go to write-back buffer until it could be serviced by the next level of memory. If there is no free entry in the write-back buffer the pipeline has to stall.
The blocks (cache-line) in the cache are loaded or removed in accordance to the replacement policy. We have implemented the random replacement policy, which will evict a cache-line from the corresponding set randomly. **You are expected to implement Least Recently Used(LRU) Policy and Pseudo LRU.**
In case of a cache miss, information about pending read/write misses are maintained in Miss Status Handling Register (MSHR). **You should also implement MSHR** for handling misses. When a miss occurs, the cache looks up the MSHR to check if a memory request to the that particular block is pending or not; If this request has been already issued (MSHR hit), the information regarding the new request will be added to that MSHR entry (please note that the number of requests for each MSHR entry is limited). In case of a MSHR miss, a new MSHR entry will be allocated. If there is no free MSHR-entry, the request will be rejected and pipeline will stall.
Prefetching is a useful technique for improvement of performance, where the core idea is to bring the data to caches before it is requested by core. We are focussing on the hardware prefetching in our simulator, where we have specialized hardware resources to detect read/write memory accesses and prefetch the data according to the past memory access patterns. The most basic form of hardware prefetching is 'Next-Line Prefetcher', which prefetches the immediately next cache line based on the past memory accesses. Typically, a prefetcher exists between the levels of memory where the prefetching is done. **Your task is to implement this prefetcher for L1 Data cache and L1 Instruction Cache.** Please note that prefetcher has to interact with MSHR, as well.

Branch Predictor
-----------------------
For implementation of Dynamic Branch Predictor, you need to implement Branch Target Buffer (BTB) to store the next target address, Return Address Stack (RAS) for handling return instructions and the predictor itself. **You are supposed to implement BTB, RAS and the branch predictor.** As an example, we have provided static branch predictor whose default decision is 'not taken'. Your implementation is supposed to perform better than our baseline implementation.

Project Evaluation
------------------------
The project is worth 50 points in total. Below are how the points are assigned to various components in the project.

Tasks to be completed 50 points + up to 5 bonus points
--------------------------------
1. Implement full bypassing in the pipeline (10 points)

2. Implement a dynamic branch predictor with a BTB and a RAS (at least as sophisticated as one of the two-level branch predictors discussed in class) (10 points)

3. Implement cache functionalities (for L1-I, L1-D, and L2 caches) and integrate them into the processor (10 points) 

4. Implement the necessary writeback buffers and MSHRs (15 points)

5. Implement LRU replacement policy and pseudo-LRU replacement policy (5 points)

6. Optional Bonus: Perform a sensitivity analysis on the size of the branch predictor (5 points)


Deliverables
----------------
- Your final simulator code via canvas
- Summary document describing branch predictor design choices and the net performance improvement over the baseline.

Description of source files:
----------------------------------
1. abstract_branch_predictor.cpp/h: Abstract class for branch predictors which should be inherited and customized for your implementation of branch predictor.
2. abstract_memory.cpp/h: Abstract class for memory hierarchy objects which you should use(inherit) for your own implementations of memory hierarchy (caches).
3. abstract_prefetcher.cpp/h: Abstract class for prefetcher which you should use(inherit) for your own implementation of next-line prefetcher (NOT REQUIRED).
4. base_memory.cpp/h: Main memory implementation based on the inheritance from abstract memory.
5. base_object.cpp/h: This is used for packet-based communication interface for handling memory requests. You do not need to change this class.
6. block.h: Definition of a single block (cache line).
7. cache.cpp/h: The cache memory interface which is inherited from the abstract_memory.
8. config_reader.cpp/h: The class for the config reader. You should not change this class at all.
9. main.cpp: Simulator interface for loading program to memory and cloning the executional behaviour.
10. mips.h: Definitions and opcodes of mips instructions.
11. next_line_prefetcher.cpp/h: Next line prefetcher class and its implementation.
12. pipe.cpp/h: MIPS pipeline interface with 5 stages.
13. repl_policy.cpp/h: Abstract class for replacement policy which should be inherited and customized for your implementation. Also random policy implementation.
14. simulator.cpp/h: Interface for simulator class.
15. static_nt_branch_predictor.cpp/h: Static not taken branch predictor.
16. Config.json: The simulation parameters

Project 1 files to modify:
--------------------------------
Here is a list of the files to implement:
* cache.cpp/h
* next_line_prefetcher.cpp/h
* repl_policy.cpp/h
* dynamic_branch_predictor.cpp/h (new file for your branch predictor)
* You may add additional files which you require. Also include those in a separate documentation file. *
Please note that you have to modify files such as simulator.cpp/h , pipe.cpp/h, base_memory.cpp/h (this is not a complete list of files that need modification, it is based on your design) to connect components and extend the functionalities of the baseline simulators.

Simulator input configuration:
--------------------------------------
You can change all parameters in the config.json file.
You can also enable debug flags (for example for memory with debugMemory) to track the events in each component.
You can also use traceMemory to generate memory access traces.

Building the simulator:
-----------------------------
make clean && make

Running the simulator:
------------------------------
./simulator /path/to/config /path/to/benchmark
Example: ./simulator config.json inputs/branch/test1.x

Running the simulator for one benchmark with comparison to baseline:
---------------------------------------------------------------------------------------------
This mode will compare the register values and the memory regions that are supposed to be changed in the benchmark with the baseline simulator. Please note that regardless of your implementation this should always match with baseline.
./run /path/to/config /path/to/benchmark
Example: ./run config.json inputs/branch/test1.x

Running the simulator for all benchmarks with comparison to baseline:
---------------------------------------------------------------------------------------------
./run /path/to/config
Example: ./run config.json

Interact with simulator:
------------------------------
When you start the simulator by running “./simulator /path/to/config /path/to/benchmark”, you can interact with the simulator by typing appropriate commands. Please refer help() function under main.cpp.
Example: SIM> ?    /* Display the help menu */
        SIM> go    /* Run program to completion */

How to debug:
-------------------
Edit config.json file to enable debug options (debugMemory, debugPipe, traceMemory, etc.).
You can also add your own debugging print-outs. Please check utils.h.
Example: DPRINTF(DEBUG_PIPE, "R%d = %08x\n", op->reg_dst, op->reg_dst_value);

