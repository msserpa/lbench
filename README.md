#  (lbench)

lbench is a benchmark to analysis the difference in latency for accessing data from local and remote memories in NUMA machines.

## Installation

     $ make
     
## Execution
     $ ./lbench <number_of_threads> <mem_per_thread_GB>
     
## Example
     $ ./lbench 32 2
     
     It will allocate 64 GB of memory for 32 threads.
