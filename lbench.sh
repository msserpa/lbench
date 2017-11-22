#!/bin/bash

PROGNAME=$(basename $0)

SYSFS_NODE="/sys/devices/system/node"
NODE_POSSIBLE_COUNT=$(ls -1d ${SYSFS_NODE}/node[0-9]* | wc -l)
#export GOMP_CPU_AFFINITY=0,100,104,120
export GOMP_CPU_AFFINITY=0-271
echo ">>>$PROGNAMiE: Running 272 threads on different nodes"
for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
#for i in `seq 0 3`; do
	echo "node $i"
	for step in `seq 1 500`; do
		numactl --membind=$i ./lbench 272 3900 -rm
		sleep 1
	done
done
