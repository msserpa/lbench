#!/bin/bash

PROGNAME=$(basename $0)

SYSFS_CPU="/sys/devices/system/cpu"
SYSFS_NODE="/sys/devices/system/node"

CPU_POSSIBLE_COUNT=$(ls -d ${SYSFS_CPU}/cpu[0-9]* | wc -l)
NODE_POSSIBLE_COUNT=$(ls -1d ${SYSFS_NODE}/node[0-9]* | wc -l)
NODE_POSSIBLE_COUNT_KNL=$(expr $NODE_POSSIBLE_COUNT / 2)

echo ">>>$PROGNAME: Running one thread on different nodes"
for ((j=0;j < ${NODE_POSSIBLE_COUNT_KNL} ;j++)); do
	core=`ls -d /sys/devices/system/node/node$j/cpu[0-9]* | head -1`
	core=`basename $core | sed s/cpu//`
	for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
		echo ">>>$PROGNAME: Running between node $i and core $core"
		numactl --membind=$i --physcpubind=$core ./lbench 1 1
	done
done
echo

if [ $NODE_POSSIBLE_COUNT -ge 4 ]; then	
	echo ">>>$PROGNAME: Running four threads on different nodes"

	core0=`ls -d /sys/devices/system/node/node0/cpu[0-9]* | head -1`
	core0=`basename $core0 | sed s/cpu//`

	core1=`ls -d /sys/devices/system/node/node1/cpu[0-9]* | head -1`
	core1=`basename $core1 | sed s/cpu//`

	core2=`ls -d /sys/devices/system/node/node2/cpu[0-9]* | head -1`
	core2=`basename $core2 | sed s/cpu//`

	core3=`ls -d /sys/devices/system/node/node3/cpu[0-9]* | head -1`
	core3=`basename $core3 | sed s/cpu//`				

	for ((i=0;i<${NODE_POSSIBLE_COUNT};i++)); do
		echo ">>>$PROGNAME: Running between node $i and cores $core0, $core1, $core2, $core3"
		numactl --membind=$i --physcpubind=$core0,core$1,core$2,core$3 ./lbench 4 1
	done
	echo
fi