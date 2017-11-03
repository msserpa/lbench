#!/bin/bash

PROGNAME=$(basename $0)

SYSFS_CPU="/sys/devices/system/cpu"
SYSFS_NODE="/sys/devices/system/node"

CPU_POSSIBLE_COUNT=$(ls -d ${SYSFS_CPU}/cpu[0-9]* | wc -l)
NODE_POSSIBLE_COUNT=$(ls -1d ${SYSFS_NODE}/node[0-9]* | wc -l)

echo ">>>$PROGNAME: Running one thread on different nodes"
for ((j=0;j < ${NODE_POSSIBLE_COUNT} ;j++)); do
	core=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | head -1`
	core=`basename $core | sed s/cpu//`
	for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
		echo ">>>$PROGNAME: Running between node $i and core $core"
		numactl --membind=$i --physcpubind=$core ./lbench 1 1
	done
done
echo

echo ">>>$PROGNAME: Running two threads on different nodes"
for ((j=0;j < ${NODE_POSSIBLE_COUNT} ;j++)); do
	core0=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n1`
	core0=`basename $core0 | sed s/cpu//`

	core1=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | cut -d" " -f1 | sort -nr | tail -n2`
	core1=`basename $core1 | sed s/cpu//`

	for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
		echo ">>>$PROGNAME: Running between node $i and cores $core0, $core1"
		numactl --membind=$i --physcpubind=$core0,core$1 ./lbench 2 1
	done
done
echo